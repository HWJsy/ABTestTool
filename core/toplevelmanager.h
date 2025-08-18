#ifndef TOPLEVELMANAGER_H
#define TOPLEVELMANAGER_H

#include <QString>
#include <QJsonArray>
#include <QJsonObject>

class TopLevelManager
{
public:
    static TopLevelManager& getInstance();

    // 加载数据
    void loadData();

    // 保存数据
    void saveData();

private:
    void loadFunctionDataFromJson(const QJsonArray& jsonArray);
    void loadGroupDataFromJson(const QJsonArray& jsonArray);
    void loadGroupMappingDataFromJson(const QJsonArray& jsonArray);
    void loadVersionDataFromJson(const QJsonArray& jsonArray);
    void loadMutexSwitchFromJson(const QJsonArray& jsonArray);

    // 将各个 Manager 数据保存到 JSON 中
    QJsonArray getFunctionDataAsJson();
    QJsonArray getGroupDataAsJson();
    QJsonArray getGroupMappingDataAsJson();
    QJsonArray getVersionDataAsJson();
    QJsonArray getMutexSwitchDataAsJson();
};

#endif // TOPLEVELMANAGER_H
