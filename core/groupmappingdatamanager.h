#ifndef GROUPMAPPINGDATAMANAGER_H
#define GROUPMAPPINGDATAMANAGER_H

#include <unordered_map>
#include <QString>
#include <QJsonArray>
class GroupMappingDataManager {
public:
    // 获取单例实例
    static GroupMappingDataManager& getInstance();

    // 获取映射
    std::unordered_map<int, int>& getGroupMapping();

    // 添加映射
    void addMapping(int groupIdOrigin, int groupIdMapping);

    // 删除映射
    void removeMapping(int groupIdOrigin);

    QJsonArray getAllAsJson();

private:

    std::unordered_map<int, int> groupMapping; // 存储映射关系
};

#endif // GROUPMAPPINGDATAMANAGER_H
