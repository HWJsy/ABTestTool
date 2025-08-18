#ifndef LOADCONFIG_H
#define LOADCONFIG_H

#include <QString>
#include <unordered_map>
#include <QList>
#include <unordered_set>

class LoadConfig {
public:
    struct FunctionData {
        int currentKeyValueId;
        QString functionDescription;
        QString functionName;
        std::unordered_map<int, std::pair<QString, QString>> keyValuePairs; // 使用 unordered_map
        QString returnType;

        // 新增：记录哪些组使用了这个函数以及使用的键值对ID
        std::unordered_map<int, int> groups; // <groupId, keyValueId>
    };

    struct GroupData {
        QString groupDescription;         // 组描述
        int groupId;                      // 组ID
    };

    struct GroupMapping {
        int mappingGroupId;
        int originGroupId;
    };

    struct VersionData {
        QString versionId;
        QString versionDescription;
        QList<int> groupIds;
        std::unordered_map<int, float> groupWeights; // 包含的分组ID以及其权重
    };

    struct MutexSwitchData {
        int id;                         // 互斥开关id
        QString description;             // 该开关的描述
        std::unordered_set<QString> functions;   // 有哪些函数设置了该开关，储存其函数名
    };

    struct ConfigData {
        QList<FunctionData> functionData;
        QList<GroupData> groupData;
        QList<GroupMapping> groupMappings;
        QList<VersionData> versionData;
        QList<MutexSwitchData> mutexSwitches;
    };

    static LoadConfig& getInstance() {
        static LoadConfig instance;
        return instance;
    }

    void loadConfig();

    // 获取ConfigData中的成员
    const QList<FunctionData>& getFunctionData() const { return configData.functionData; }
    const QList<GroupData>& getGroupData() const { return configData.groupData; }
    const QList<GroupMapping>& getGroupMappings() const { return configData.groupMappings; }
    const QList<VersionData>& getVersionData() const { return configData.versionData; }
    const QList<MutexSwitchData>& getMutexSwitchData() const { return configData.mutexSwitches; }

private:
    // 存储配置数据的成员变量
    ConfigData configData;
};

#endif // LOADCONFIG_H
