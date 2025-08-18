#include "LoadConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
void LoadConfig::loadConfig() {
    QSettings settings("App");
    QString configFilePath = settings.value("configFilePath").toString();

    if (configFilePath.isEmpty()) {
        qDebug() << "Config file path is empty!";
        return;
    }

    // 打开文件
    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file:" << configFilePath;
        return;
    }

    // 读取JSON
    QByteArray fileData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(fileData);
    if (document.isNull()) {
        qDebug() << "Failed to parse JSON.";
        return;
    }

    // 获取 JSON 对象
    QJsonObject rootObj = document.object();

    QString exportPath = rootObj["cppExportPath"].toString();
    if(!exportPath.isEmpty()) {
        QFileInfo configFileInfo(configFilePath);
        QDir configDir(configFileInfo.absolutePath());

        // 将相对路径转换为绝对路径
        exportPath = configDir.absoluteFilePath(exportPath);
    }
    settings.setValue("cppExportPath", exportPath);

    // 清空原有数据
    configData.functionData.clear();
    configData.groupData.clear();
    configData.groupMappings.clear();
    configData.versionData.clear();
    configData.mutexSwitches.clear();

    // 读取 functionData
    QJsonArray functionDataArray = rootObj["functionData"].toArray();
    for (const QJsonValue &value : functionDataArray) {
        QJsonObject functionObj = value.toObject();
        FunctionData functionData;
        functionData.currentKeyValueId = functionObj["currentKeyValueId"].toInt();
        functionData.functionDescription = functionObj["functionDescription"].toString();
        functionData.functionName = functionObj["functionName"].toString();
        functionData.returnType = functionObj["returnType"].toString();
        QJsonArray keyValuePairsArray = functionObj["keyValuePairs"].toArray();
        for (const QJsonValue &kvValue : keyValuePairsArray) {
            QJsonObject keyValueObj = kvValue.toObject();
            int id = keyValueObj["id"].toInt();  // 获取 ID
            QJsonObject keyValuePairObj = keyValueObj["keyValuePair"].toObject();
            QString key = keyValuePairObj["key"].toString();  // 获取 key
            QString value = keyValuePairObj["value"].toString();  // 获取 value
            functionData.keyValuePairs[id] = std::pair<QString, QString>(key, value);  // 插入到 unordered_map
        }
        configData.functionData.append(functionData);
    }

    // 读取 groupData
    QJsonArray groupDataArray = rootObj["groupData"].toArray();
    for (const QJsonValue &value : groupDataArray) {
        QJsonObject groupObj = value.toObject();
        GroupData groupData;
        groupData.groupId = groupObj["groupId"].toInt();
        groupData.groupDescription = groupObj["groupDescription"].toString();

        // 获取该组的 functions 字段
        QJsonObject functionsObj = groupObj["functions"].toObject();
        // 遍历该组的每个函数
        for (auto it = functionsObj.begin(); it != functionsObj.end(); ++it) {
            QString functionName = it.key();  // 获取函数名
            int keyValueId = it.value().toInt();  // 获取该函数使用的键值对 ID

            // 在 functionData 中查找对应的函数，并填充其 groups
            for (auto &function : configData.functionData) {
                if (function.functionName == functionName) {
                    // 如果函数返回类型为 "bool"，则只在keyValueId 为 1时记录该 group 和 keyValueId
                    if (function.returnType == "bool" && keyValueId == 0) {
                        continue;
                    }
                    // 对于其他返回类型，直接记录组 ID 和 keyValueId
                    else {
                        function.groups[groupData.groupId] = keyValueId;
                    }
                }
            }
        }
        configData.groupData.append(groupData);
    }

    // 读取 groupMappings
    QJsonArray groupMappingsArray = rootObj["groupMappings"].toArray();
    for (const QJsonValue &value : groupMappingsArray) {
        QJsonObject groupMappingObj = value.toObject();
        GroupMapping groupMapping;
        groupMapping.mappingGroupId = groupMappingObj["mappingGroupId"].toInt();
        groupMapping.originGroupId = groupMappingObj["originGroupId"].toInt();

        configData.groupMappings.append(groupMapping);
    }

    // 读取 versionData
    QJsonArray versionDataArray = rootObj["versionData"].toArray();
    for (const QJsonValue &value : versionDataArray) {
        QJsonObject versionObj = value.toObject();
        VersionData versionData;
        versionData.versionId = versionObj["versionId"].toString();
        versionData.versionDescription = versionObj["versionDescription"].toString();

        QJsonArray groupIdsArray = versionObj["groupIds"].toArray();
        for (const QJsonValue &groupIdValue : groupIdsArray) {
            versionData.groupIds.append(groupIdValue.toInt());
        }

        QJsonObject groupWeightsObj = versionObj["groupWeights"].toObject();
        for (const QString& groupIdStr : groupWeightsObj.keys()) {
            int groupId = groupIdStr.toInt();
            float weight = groupWeightsObj[groupIdStr].toDouble();
            versionData.groupWeights[groupId] = weight;
        }

        configData.versionData.append(versionData);
    }

    QJsonArray mutexSwitcherDataArray = rootObj["mutexSwitches"].toArray();
    for(const QJsonValue& value: mutexSwitcherDataArray) {
        QJsonObject mutexSwitchObject = value.toObject();
        MutexSwitchData mutexSwitch;
        mutexSwitch.id = mutexSwitchObject["id"].toInt();
        mutexSwitch.description = mutexSwitchObject["description"].toString();

        QJsonArray functions = mutexSwitchObject["functions"].toArray();
        for (const auto& function: functions) {
            mutexSwitch.functions.insert(function.toString());
        }
        configData.mutexSwitches.append(mutexSwitch);
    }
}
