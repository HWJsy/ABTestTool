#include "groupdatamanager.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include "functiondatamanager.h"
#include "toplevelmanager.h"
void GroupDataManager::addGroup(const QString& groupDescription)
{
    // 为新分组分配 ID
    GroupManageData newGroup;
    newGroup.groupDescription = groupDescription;
    newGroup.groupId = count++;  // ID 为当前 count++

    //设置默认值
    for(auto function: FunctionDataManager::getInstance().getAllFunctionData())
    {
        newGroup.functions[function.functionName] = function.currentKeyValueId;
    }

    // 将新分组添加到列表中
    groupList.append(newGroup);

    // 保存数据到文件
    TopLevelManager::getInstance().saveData();
}

GroupManageData GroupDataManager::getGroupById(int groupId)
{
    for (const GroupManageData& group : groupList) {
        if (group.groupId == groupId) {
            return group;
        }
    }
    return GroupManageData();
}

QList<GroupManageData> GroupDataManager::getAllGroupData()
{
    return groupList;  // 返回所有的分组数据
}


void GroupDataManager::updateGroupDescription(int groupId, const QString& groupDescription)
{
    for (int i = 0; i < groupList.size(); ++i) {
        if (groupList[i].groupId == groupId) {
            // 修改 groupDescription
            groupList[i].groupDescription = groupDescription;
            TopLevelManager::getInstance().saveData(); // 修改后保存数据
            return;
        }
    }
}

void GroupDataManager::updateGroupFunction(int groupId, const QString& functionName) // 函数的返回类型是bool时，调用这个函数
{
    groupId--;
    if(groupList[groupId].functions[functionName] == 1) {
        groupList[groupId].functions[functionName] = 0;
    } else {
        groupList[groupId].functions[functionName] = 1;
    }
    TopLevelManager::getInstance().saveData();
}

void GroupDataManager::updateGroupFunction(int groupId, const QString& functionName, const int& id) // 函数的返回类型不是bool时，调用这个函数
{
    groupId--;
    groupList[groupId].functions[functionName] = id;
    TopLevelManager::getInstance().saveData();
}


void GroupDataManager::removeGroup(int groupId)
{
    for (int i = 0; i < groupList.size(); ++i) {
        if (groupList[i].groupId == groupId) {
            groupList.removeAt(i);
            TopLevelManager::getInstance().saveData();  // 删除后保存数据
            return;
        }
    }
}

QList<GroupManageData>& GroupDataManager::getGroupList()
{
    return groupList;
}

QJsonArray GroupDataManager::getAllAsJson()
{
    QJsonArray jsonArray;

    // 将 groupList 转换为 JSON 格式
    for (const GroupManageData& group : groupList) {
        QJsonObject jsonObject;
        jsonObject["groupDescription"] = group.groupDescription;
        jsonObject["groupId"] = group.groupId;

        QJsonObject functionsObject;
        for (const auto& function : group.functions) {
            functionsObject[function.first] = function.second;
        }
        jsonObject["functions"] = functionsObject;

        jsonArray.append(jsonObject);
    }
    return jsonArray;
}
