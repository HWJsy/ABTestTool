#include "toplevelmanager.h"
#include "functiondatamanager.h"
#include "groupdatamanager.h"
#include "groupmappingdatamanager.h"
#include "versiondatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>

TopLevelManager& TopLevelManager::getInstance()
{
    static TopLevelManager instance;
    return instance;
}

void TopLevelManager::loadData()
{
    // 获取用户设置的路径
    QSettings settings("App");
    QString savePath = settings.value("configFilePath").toString();

    QFile file(savePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject rootObj = doc.object();

        // 分发到各个子 Manager 类
        loadFunctionDataFromJson(rootObj["functionData"].toArray());
        loadGroupDataFromJson(rootObj["groupData"].toArray());
        loadGroupMappingDataFromJson(rootObj["groupMappings"].toArray());
        loadVersionDataFromJson(rootObj["versionData"].toArray());
        loadMutexSwitchFromJson(rootObj["mutexSwitches"].toArray());

        QString cppExportPath = rootObj["cppExportPath"].toString();
        if (!cppExportPath.isEmpty()) {
            QFileInfo configFileInfo(savePath);
            QDir configDir(configFileInfo.absolutePath());

            // 将 cppExportPath 转换为绝对路径
            QString absolutePath = configDir.absoluteFilePath(cppExportPath);
            settings.setValue("cppExportPath", absolutePath);
        }
    } else {
        qWarning() << "无法打开文件！";
    }
}

void TopLevelManager::saveData()
{
    // 获取用户设置的路径
    QSettings settings("App");
    QString savePath = settings.value("configFilePath").toString();

    if (savePath.isEmpty()) {
        qWarning() << "未设置路径，无法保存文件！";
        return;
    }

    // 获取各个子 Manager 的数据
    QJsonObject allDataJson;

    allDataJson["functionData"] = getFunctionDataAsJson();
    allDataJson["groupData"] = getGroupDataAsJson();
    allDataJson["groupMappings"] = getGroupMappingDataAsJson();
    allDataJson["versionData"] = getVersionDataAsJson();
    allDataJson["mutexSwitches"] = getMutexSwitchDataAsJson();
    // 获取配置文件路径
    QString cppExportPath = settings.value("cppExportPath").toString();
    if (!cppExportPath.isEmpty()) {
        QFileInfo configFileInfo(savePath);

        // 将导出的路径转为相对路径
        QDir configDir(configFileInfo.absolutePath());
        QString relativePath = configDir.relativeFilePath(cppExportPath);
        allDataJson["cppExportPath"] = relativePath;
    }

    // 保存到文件
    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(allDataJson);
        file.write(doc.toJson());
        file.close();
    } else {
        qWarning() << "无法保存数据到文件！";
    }
}

void TopLevelManager::loadFunctionDataFromJson(const QJsonArray& jsonArray)
{
    FunctionDataManager::getInstance().getFunctionDataList().clear();
    // 加载 FunctionDataManager 数据
    for (const QJsonValue& value : jsonArray) {
        QJsonObject functionObject = value.toObject();
        FunctionData data;
        data.functionName = functionObject["functionName"].toString();
        data.functionDescription = functionObject["functionDescription"].toString();
        data.returnType = functionObject["returnType"].toString();

        data.currentKeyValueId = functionObject["currentKeyValueId"].toInt();

        QJsonArray keyValuePairsArray = functionObject["keyValuePairs"].toArray();
        for (const QJsonValue& pairValue : keyValuePairsArray) {
            QJsonObject pairObject = pairValue.toObject();

            int id = pairObject["id"].toInt();

            QJsonObject valueObject = pairObject["keyValuePair"].toObject();
            QString key = valueObject["key"].toString();
            QString value = valueObject["value"].toString();

            data.keyValuePairs[id] = std::pair(key, value);
        }


        FunctionDataManager::getInstance().getFunctionDataList().append(data);
    }

}

void TopLevelManager::loadGroupDataFromJson(const QJsonArray& jsonArray)
{
    GroupDataManager::getInstance().count = 1;
    GroupDataManager::getInstance().getGroupList().clear();
    // 加载 GroupDataManager 数据
    for (const QJsonValue& value : jsonArray) {
        QJsonObject groupObject = value.toObject();
        GroupManageData group;
        group.groupDescription = groupObject["groupDescription"].toString();
        group.groupId = groupObject["groupId"].toInt();
        QJsonObject functionsObject = groupObject["functions"].toObject();
        for (const QString& key : functionsObject.keys()) {
            group.functions[key] = functionsObject[key].toInt();
        }
        GroupDataManager::getInstance().getGroupList().append(group);
        GroupDataManager::getInstance().count++;
    }
}

void TopLevelManager::loadGroupMappingDataFromJson(const QJsonArray& jsonArray)
{
    GroupMappingDataManager::getInstance().getGroupMapping().clear();
    // 加载 GroupMappingDataManager 数据
    for (const QJsonValue& value : jsonArray) {
        QJsonObject mappingObject = value.toObject();
        int originGroupId = mappingObject["originGroupId"].toInt();
        int mappingGroupId = mappingObject["mappingGroupId"].toInt();
        GroupMappingDataManager::getInstance().getGroupMapping()[originGroupId] = mappingGroupId;
    }
}

void TopLevelManager::loadVersionDataFromJson(const QJsonArray& jsonArray)
{
    VersionDataManager::getInstance().getVersionDataList().clear();
    // 加载 VersionDataManager 数据
    for (const QJsonValue& value : jsonArray) {
        QJsonObject versionObject = value.toObject();
        VersionManageData version;
        version.versionId = versionObject["versionId"].toString();
        version.versionDescription = versionObject["versionDescription"].toString();
        version.isAverage = versionObject["isAverage"].toBool();

        QJsonArray groupIdsArray = versionObject["groupIds"].toArray();
        for (const QJsonValue& groupIdValue : groupIdsArray) {
            version.groupIds.append(groupIdValue.toInt());
        }

        QJsonObject groupWeightsObj = versionObject["groupWeights"].toObject();
        for (const QString& groupIdStr : groupWeightsObj.keys()) {
            int groupId = groupIdStr.toInt();
            float weight = groupWeightsObj[groupIdStr].toDouble();
            version.groupWeights[groupId] = weight;
        }

        VersionDataManager::getInstance().getVersionDataList().append(version);
    }
}

void TopLevelManager::loadMutexSwitchFromJson(const QJsonArray& jsonArray)
{
    FunctionDataManager::getInstance().getMutexSwitches().clear();
    FunctionDataManager::getInstance().countMutexSwitch = 0;

    for(const QJsonValue& value: jsonArray) {
        QJsonObject mutexSwitchObject = value.toObject();
        MutexSwitchData mutexSwitch;
        mutexSwitch.id = mutexSwitchObject["id"].toInt();
        mutexSwitch.description = mutexSwitchObject["description"].toString();

        QJsonArray functions = mutexSwitchObject["functions"].toArray();
        for (const auto& function: functions) {
            mutexSwitch.functions.insert(function.toString());
        }

        FunctionDataManager::getInstance().getMutexSwitches().append(mutexSwitch);
        FunctionDataManager::getInstance().countMutexSwitch++;
    }
}



QJsonArray TopLevelManager::getFunctionDataAsJson()
{
    return FunctionDataManager::getInstance().getAllAsJson();
}

QJsonArray TopLevelManager::getGroupDataAsJson()
{
    return GroupDataManager::getInstance().getAllAsJson();
}

QJsonArray TopLevelManager::getGroupMappingDataAsJson()
{
    return GroupMappingDataManager::getInstance().getAllAsJson();
}

QJsonArray TopLevelManager::getVersionDataAsJson()
{
    return VersionDataManager::getInstance().getAllAsJson();
}

QJsonArray TopLevelManager::getMutexSwitchDataAsJson()
{
    return FunctionDataManager::getInstance().getMutexSwitchesAsJson();
}
