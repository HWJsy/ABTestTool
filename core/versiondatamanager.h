#ifndef VERSIONDATAMANAGER_H
#define VERSIONDATAMANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QJsonArray>
struct VersionManageData {
    QString versionId;               // 版本ID
    QString versionDescription;      // 版本描述
    std::unordered_map<int, float> groupWeights; // 包含的分组ID以及其权重
    QList<int> groupIds;
    bool isAverage;                  // 是否平均权重
};

class VersionDataManager {
public:
    static VersionDataManager& getInstance() {
        static VersionDataManager instance;
        return instance;
    }

    // 增：添加新版本
    void addVersionData(const QString& versionId, const QString& versionDescription);

    // 查：获取版本数据
    VersionManageData* getVersionData(const QString& versionId);
    QList<VersionManageData>& getAllVersionData();

    // 改：修改 versionDescription
    void updateVersionDescription(const QString& versionId, const QString& versionDescription);

    // 改：修改 groupIds（添加或删除 groupId）
    void updateVersionGroupIds(const QString& versionId, int groupId);

    // 删：删除版本数据
    void removeVersionData(const QString& versionId);

    QList<VersionManageData>& getVersionDataList();

    QJsonArray getAllAsJson();

private:
    QList<VersionManageData> versionDataList;
};

#endif // VERSIONDATAMANAGER_H
