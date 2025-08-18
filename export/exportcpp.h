#ifndef EXPORTCPP_H
#define EXPORTCPP_H

#include <QString>
#include <QList>
#include <QCoreApplication>
#include <QDir>
#include "loadConfig.h"
class ExportCpp {
public:
    // 获取单例实例
    static ExportCpp& getInstance() {
        static ExportCpp instance; // C++11中的局部静态变量保证线程安全和惰性初始化
        return instance;
    }

    // 设置模板文件路径（默认初始化为 "../template/"）
    void setTemplateFilePath(const QString& path) { templateFilePath = path; }

    // 获取模板文件路径
    QString getTemplateFilePath() const { return templateFilePath; }

    // 导出头文件，生成对应的函数
    bool exportHeader();

    // 导出cpp文件，生成对应的函数
    bool exportCpp();

    QString checkMutex();

private:
    ExportCpp() {
        QString appDir = QCoreApplication::applicationDirPath();

        // 构造 template 文件夹的路径
        QDir dir(appDir);
        dir.cd("../Resources/template");
        templateFilePath = dir.absolutePath();  // 获取绝对路径
    }

    // 从配置中生成对应的函数声明
    QString generateFunctionDeclarations() const;

    // 替换分组配置数据
    void replaceGroupConfigData(QString &templateContent);

    // 替换分组映射规则
    void replaceGroupMapping(QString &templateContent);

    // 函数模板替换相关
    QString generateBoolFunctionSmall(const LoadConfig::FunctionData &functionData) const;
    QString generateBoolFunctionMedium(const LoadConfig::FunctionData &functionData) const;
    QString generateBoolFunctionLarge(const LoadConfig::FunctionData &functionData) const;
    QString generateIntFunction(const LoadConfig::FunctionData &functionData) const;
    QString generateFloatFunction(const LoadConfig::FunctionData &functionData) const;
    QString generateStringFunction(const LoadConfig::FunctionData &functionData) const;

    // 获取合适的函数模板
    QString generateFunction(const QList<LoadConfig::FunctionData> &functionData) const;

    QString templateFilePath;  // 模板文件所在的文件夹路径
};

#endif // EXPORTCPP_H
