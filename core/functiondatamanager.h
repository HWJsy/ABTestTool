#ifndef FUNCTIONDATAMANAGER_H
#define FUNCTIONDATAMANAGER_H

#include <QList>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <unordered_set>
using namespace std;
struct FunctionData {
    QString functionDescription;                // 功能名称
    QString functionName;                       // 函数名称
    QString returnType;
    int currentKeyValueId;
    unordered_map<int, pair<QString, QString>> keyValuePairs;
};

struct MutexSwitchData {
    int id;                         // 互斥开关id
    QString description;             // 该开关的描述
    unordered_set<QString> functions;   // 有哪些函数设置了该开关，储存其函数名
};

class FunctionDataManager
{
public:
    static FunctionDataManager& getInstance() {
        static FunctionDataManager instance;
        return instance;
    }

    // 增：添加新的功能
    void addFunctionData(const FunctionData &data);
    void addMutexSwitch(const QString& description);

    // 查：获取所有功能
    QList<FunctionData> getAllFunctionData() const;
    FunctionData& getFunctionByName(QString functionName);

    // 删：删除指定的功能
    void removeFunctionData(const QString &functionName);

    // 改：修改指定功能的信息
    void updateFunctionData(const QString &functionName, const FunctionData &newData);
    void updateMutexSwitch(const int& id, const QString& description);

    QList<FunctionData>& getFunctionDataList();

    QList<MutexSwitchData>& getMutexSwitches();

    QJsonArray getAllAsJson();

    QJsonArray getMutexSwitchesAsJson();

    int countMutexSwitch;

private:

    QList<FunctionData> functionDataList;  // 存储所有功能数据
    QList<MutexSwitchData> mutexSwitches;
};

#endif // FUNCTIONDATAMANAGER_H
