#include "versiondatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include "toplevelmanager.h"
// 增：添加新版本
void VersionDataManager::addVersionData(const QString& versionId, const QString& versionDescription)
{
    VersionManageData data;
    data.versionId = versionId;
    data.versionDescription = versionDescription;
    versionDataList.append(data);

    // 每次增、删、改后保存到文件
    TopLevelManager::getInstance().saveData();
}

// 查：获取版本数据
VersionManageData* VersionDataManager::getVersionData(const QString& versionId)
{
    for(auto& it: versionDataList)
    {
        if(it.versionId == versionId)
            return &it;
    }
    return nullptr;  // 如果没有找到该版本ID
}

QList<VersionManageData>&  VersionDataManager::getAllVersionData()
{
    return versionDataList;
}

// 改：修改 versionDescription
void VersionDataManager::updateVersionDescription(const QString& versionId, const QString& versionDescription)
{
    VersionManageData *data = getVersionData(versionId);
    if (data) {
        data->versionDescription = versionDescription;
        TopLevelManager::getInstance().saveData();
    }
}

// 改：修改 groupIds（添加或删除 groupId）
void VersionDataManager::updateVersionGroupIds(const QString& versionId, int groupId)
{
    VersionManageData* data = getVersionData(versionId);
    if (data) {
        if (data->groupIds.contains(groupId)) {
            // 如果 groupIds 已经包含 groupId，删除它
            data->groupIds.removeAll(groupId);
        } else {
            // 否则添加该 groupId
            data->groupIds.append(groupId);
        }
        TopLevelManager::getInstance().saveData();
    }
}

// 删：删除版本数据
void VersionDataManager::removeVersionData(const QString& versionId)
{
    for (int i = 0; i < versionDataList.size(); ++i) {
        if (versionDataList[i].versionId == versionId) {
            versionDataList.removeAt(i);
            TopLevelManager::getInstance().saveData();
            return;
        }
    }
}

// 导出：保存数据到文件
QList<VersionManageData>& VersionDataManager::getVersionDataList()
{
    return versionDataList;
}

// 导出：保存数据到文件
QJsonArray VersionDataManager::getAllAsJson()
{
    QJsonArray versionArray;
    for (auto it = versionDataList.begin(); it != versionDataList.end(); ++it) {
        QJsonObject versionObj;

        versionObj["versionId"] = it->versionId;
        versionObj["versionDescription"] = it->versionDescription;

        QJsonArray groupIdsArray;
        for (int groupId : it->groupIds) {
            groupIdsArray.append(groupId);
        }

        versionObj["groupIds"] = groupIdsArray;

        versionObj["isAverage"] = it->isAverage;

        QJsonObject groupWeightsObj;
        for (const auto &weight : it->groupWeights) {
            groupWeightsObj[QString::number(weight.first)] = weight.second;
        }
        versionObj["groupWeights"] = groupWeightsObj;

        versionArray.append(versionObj);
    }
    return versionArray;
}
