#include "groupmappingdatamanager.h"
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include "toplevelmanager.h"
// 获取单例实例
GroupMappingDataManager& GroupMappingDataManager::getInstance() {
    static GroupMappingDataManager instance;
    return instance;
}

// 获取映射
std::unordered_map<int, int>& GroupMappingDataManager::getGroupMapping() {
    return groupMapping;
}

// 添加映射
void GroupMappingDataManager::addMapping(int groupIdOrigin, int groupIdMapping) {
    groupMapping[groupIdOrigin] = groupIdMapping;  // 添加映射关系
    TopLevelManager::getInstance().saveData();
}

void GroupMappingDataManager::removeMapping(int groupIdOrigin) {
    auto it = groupMapping.find(groupIdOrigin);
    if (it != groupMapping.end()) {
        groupMapping.erase(it);  // 删除映射关系
        TopLevelManager::getInstance().saveData();
    } else {
        qWarning() << "映射关系中没有找到对应的分组ID：" << groupIdOrigin;
    }
}

// 导出
QJsonArray GroupMappingDataManager::getAllAsJson()
{
    // 将映射关系转化为 JSON
    QJsonArray jsonArray;
    for (const auto& pair : groupMapping) {
        QJsonObject mappingObject;
        mappingObject["originGroupId"] = pair.first;
        mappingObject["mappingGroupId"] = pair.second;
        jsonArray.append(mappingObject);
    }
    return jsonArray;
}
