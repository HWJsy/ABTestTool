#ifndef GROUPDATAMANAGER_H
#define GROUPDATAMANAGER_H

#include <QString>
#include <QList>
#include <QJsonArray>
struct GroupManageData {
    QString groupDescription;         // 组描述
    int groupId;                      // 组ID
    std::unordered_map<QString, int> functions;     // <函数名, 键值对id>
};

class GroupDataManager {
public:
    // 获取单例实例
    static GroupDataManager& getInstance(){
        static GroupDataManager instance;
        return instance;
    }

    // 增
    void addGroup(const QString& groupDescription);

    QList<GroupManageData> getAllGroupData();
    GroupManageData getGroupById(int groupId);

    // 改
    void updateGroupDescription(int groupId, const QString& groupDescription);
    void updateGroupFunction(int groupId, const QString& groupFunction);
    void updateGroupFunction(int groupId, const QString& groupFunction, const int& id);
    // 删
    void removeGroup(int groupId);

    QList<GroupManageData>& getGroupList();

    QJsonArray getAllAsJson();

    int count;
private:

    // 存储所有分组数据
    QList<GroupManageData> groupList;
};

#endif // GROUPDATAMANAGER_H
