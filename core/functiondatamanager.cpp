#include "functiondatamanager.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>
#include "toplevelmanager.h"
// 增：添加新的功能
void FunctionDataManager::addFunctionData(const FunctionData &data)
{
    // 将 FunctionData 添加到 list 中并保存到文件
    functionDataList.append(data);
    TopLevelManager::getInstance().saveData();
}

void FunctionDataManager::addMutexSwitch(const QString& description)
{
    MutexSwitchData mutexSwitch;
    mutexSwitch.description = description;
    mutexSwitch.id = countMutexSwitch++;
    mutexSwitches.append(mutexSwitch);
    TopLevelManager::getInstance().saveData();
}


// 查：获取所有功能
QList<FunctionData> FunctionDataManager::getAllFunctionData() const
{
    return functionDataList;
}

FunctionData& FunctionDataManager::getFunctionByName(QString functionName)
{
    for(auto& it: functionDataList) {
        if(it.functionName == functionName) {
            return it;
        }
    }

    // 没有找到，返回一个默认构造的 FunctionData
    qWarning() << "FunctionDataManager::getFunctionByName 没有找到对应的功能";
    static FunctionData defaultFunction;
    return defaultFunction;
}

// 删：删除指定的功能
void FunctionDataManager::removeFunctionData(const QString &functionName)
{
    for (int i = 0; i < functionDataList.size(); ++i) {
        if (functionDataList[i].functionName == functionName) {
            functionDataList.removeAt(i);
            TopLevelManager::getInstance().saveData();
            return;
        }
    }
}

// 改：修改指定功能的信息
void FunctionDataManager::updateFunctionData(const QString &functionName, const FunctionData &newData)
{
    for (int i = 0; i < functionDataList.size(); ++i) {
        if (functionDataList[i].functionName == functionName) {
            functionDataList[i] = newData;
            TopLevelManager::getInstance().saveData();
            return;
        }
    }
}

void FunctionDataManager::updateMutexSwitch(const int& id, const QString& description)
{
    mutexSwitches[id].description = description;
}

QList<FunctionData>& FunctionDataManager::getFunctionDataList()
{
    return functionDataList;
}

QList<MutexSwitchData>& FunctionDataManager::getMutexSwitches()
{
    return mutexSwitches;
}

// 持久化：保存功能数据到文件
QJsonArray FunctionDataManager::getAllAsJson()
{
    QJsonArray jsonArray;

    for (const FunctionData &data : functionDataList) {
        QJsonObject jsonObject;
        jsonObject["functionName"] = data.functionName;
        jsonObject["functionDescription"] = data.functionDescription;
        jsonObject["returnType"] = data.returnType;
        jsonObject["currentKeyValueId"] = data.currentKeyValueId;

        QJsonArray keyValuePairsArray;
        for (const auto& pair : data.keyValuePairs) {
            QJsonObject pairObject;
            pairObject["id"] = pair.first;

            QJsonObject valueObject;
            valueObject["key"] = pair.second.first;
            valueObject["value"] = pair.second.second;

            pairObject["keyValuePair"] = valueObject;

            keyValuePairsArray.append(pairObject);
        }
        jsonObject["keyValuePairs"] = keyValuePairsArray;

        jsonArray.append(jsonObject);
    }
    return jsonArray;
}

QJsonArray FunctionDataManager::getMutexSwitchesAsJson()
{
    QJsonArray jsonArray;

    for(const auto& mutexSwitch: mutexSwitches) {
        QJsonObject jsonObject;
        jsonObject["id"] = mutexSwitch.id;
        jsonObject["description"] = mutexSwitch.description;

        QJsonArray functions;
        for(const auto& it: mutexSwitch.functions) {
            functions.append(it);
        }
        jsonObject["functions"] = functions;

        jsonArray.append(jsonObject);
    }
    return jsonArray;
}

