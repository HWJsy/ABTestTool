#include "exportcpp.h"
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QMessageBox>
#include "loadConfig.h"

bool ExportCpp::exportHeader()
{
    // 查找模板文件夹中的 .hpp 文件
    QDir templateDir(templateFilePath);
    templateDir.setNameFilters({"*.hpp"});
    templateDir.setFilter(QDir::Files);

    // 获取文件夹中的文件列表
    QStringList files = templateDir.entryList();
    if (files.isEmpty())
    {
        qDebug() << "No template .hpp file found in" << templateFilePath;
        return 0;
    }

    QString templateFileName = files.first();
    QString templateFileFullPath = templateFilePath + "/" + templateFileName;

    // 打开模板文件
    QFile templateFile(templateFileFullPath);
    if (!templateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open template file:" << templateFileFullPath;
        return 0;
    }

    // 读取模板文件内容
    QTextStream in(&templateFile);
    QString templateContent = in.readAll();
    templateFile.close();

    // 获取生成的功能开关函数
    QString functionDeclarations = generateFunctionDeclarations();

    // 替换模板中的占位符
    templateContent.replace("    @函数声明", "    //自动生成的功能开关函数\n" + functionDeclarations);

    // 获取模板文件名
    QFileInfo templateFileInfo(templateFileFullPath);
    QString outputFileName = templateFileInfo.fileName(); // 使用模板文件的名字作为导出的文件名

    // 从 QSettings 获取导出路径
    QSettings settings("App");
    QString savedPath = settings.value("cppExportPath").toString();

    // 生成导出的完整路径
    QString outputFilePath = savedPath + "/" + outputFileName;

    // 将替换后的内容写入输出文件
    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open output file:" << outputFilePath;
        return 0;
    }

    QTextStream out(&outputFile);
    out << templateContent;
    outputFile.close();

    qDebug() << "Header file generated at" << outputFilePath;
    return 1;
}

bool ExportCpp::exportCpp()
{
    // 查找模板文件夹中的 .cpp 文件
    QDir templateDir(templateFilePath);
    templateDir.setNameFilters({"*.cpp"});
    templateDir.setFilter(QDir::Files);

    // 获取文件夹中的文件列表
    QStringList files = templateDir.entryList();
    if (files.isEmpty())
    {
        qDebug() << "No template .cpp file found in" << templateFilePath;
        return 0;
    }

    // 假设模板文件夹下只有一个 .cpp 文件，获取文件名
    QString templateFileName = files.first();
    QString templateFileFullPath = templateFilePath + "/" + templateFileName;

    // 打开模板文件
    QFile templateFile(templateFileFullPath);
    if (!templateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open template file:" << templateFileFullPath;
        return 0;
    }

    // 读取模板文件内容
    QTextStream in(&templateFile);
    QString templateContent = in.readAll();
    templateFile.close();

    // 替换分组配置数据和分组映射规则
    replaceGroupConfigData(templateContent);
    replaceGroupMapping(templateContent);

    // 生成函数并替换到模版中
    const auto &functionData = LoadConfig::getInstance().getFunctionData();
    QString functionImplementations = generateFunction(functionData); // 生成所有的函数实现
    templateContent.replace("@函数实现", "//自动生成的功能开关函数\n" + functionImplementations);

    // 获取模板文件名
    QFileInfo templateFileInfo(templateFileFullPath);
    QString outputFileName = templateFileInfo.fileName(); // 使用模板文件的名字作为导出的文件名

    // 从 QSettings 获取导出路径
    QSettings settings("App");
    QString savedPath = settings.value("cppExportPath").toString();

    // 生成导出的完整路径
    QString outputFilePath = savedPath + "/" + outputFileName;

    // 将替换后的内容写入输出文件
    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open output file:" << outputFilePath;
        return 0;
    }

    QTextStream out(&outputFile);
    out << templateContent;
    outputFile.close();

    qDebug() << "CPP file generated at" << outputFilePath;
    return 1;
}

// 辅助函数：根据起始 '{' 找到配对的 '}'
int findClosingBrace(const QString &templateContent, int startBracePos)
{
    int braceCount = 1; // 计数 '{' 和 '}' 配对
    int pos = startBracePos + 1;

    // 遍历直到找到配对的 '}'
    while (pos < templateContent.length())
    {
        if (templateContent[pos] == '{')
        {
            braceCount++;
        }
        else if (templateContent[pos] == '}')
        {
            braceCount--;
            if (braceCount == 0)
            {
                return pos; // 找到配对的 '}'
            }
        }
        pos++;
    }

    return -1; // 如果找不到配对的 '}'，返回 -1
}

QString ExportCpp::generateFunction(const QList<LoadConfig::FunctionData> &functionData) const
{
    QString functionImplementations;

    // 遍历 FunctionData
    for (auto &function : functionData)
    {
        // 根据返回类型生成函数
        if (function.returnType == "bool")
        {
            // 对于 bool 类型，检查 groups 的大小，决定调用哪个函数
            if (function.groups.size() <= 1)
            {
                functionImplementations += generateBoolFunctionSmall(function);
            }
            else if (function.groups.size() > 1 && function.groups.size() <= 10)
            {
                functionImplementations += generateBoolFunctionMedium(function);
            }
            else
            {
                functionImplementations += generateBoolFunctionLarge(function);
            }
        }
        else if (function.returnType == "int")
        {
            functionImplementations += generateIntFunction(function);
        }
        else if (function.returnType == "float")
        {
            functionImplementations += generateFloatFunction(function);
        }
        else if (function.returnType == "string")
        {
            functionImplementations += generateStringFunction(function);
        }
        functionImplementations += "\n";
    }

    return functionImplementations;
}

QString ExportCpp::generateFunctionDeclarations() const
{
    QString declarations;

    const auto &functionData = LoadConfig::getInstance().getFunctionData();

    // 遍历函数数据，生成对应的函数声明
    for (const auto &function : functionData)
    {
        // 根据返回类型生成函数声明
        if (function.returnType == "bool")
        {
            declarations += "    bool " + function.functionName + "();\n";
        }
        else if (function.returnType == "int")
        {
            declarations += "    int " + function.functionName + "();\n";
        }
        else if (function.returnType == "float")
        {
            declarations += "    float " + function.functionName + "();\n";
        }
        else if (function.returnType == "string")
        {
            declarations += "    std::string " + function.functionName + "();\n";
        }
    }

    return declarations;
}

void ExportCpp::replaceGroupConfigData(QString &templateContent)
{
    // 从配置中获取版本数据
    const auto &versionData = LoadConfig::getInstance().getVersionData();

    // 查找版本 ID 最大的版本（最新版本）
    LoadConfig::VersionData latestVersion;
    for (const auto &version : versionData)
    {
        if (latestVersion.versionId.isEmpty() || version.versionId > latestVersion.versionId)
        {
            latestVersion = version;
        }
    }

    // 从最新版本的 groupIds 中获取启用的分组 ID
    QList<int> enabledGroupIds = latestVersion.groupIds;

    // 从配置中获取所有分组数据
    const auto &groupData = LoadConfig::getInstance().getGroupData();

    // 获取启用的分组 ID、描述以及权重
    QStringList groups;
    QStringList groupDescriptions;
    QStringList groupWeights;

    std::map<int, float> groupMap; // 用于填充 groups

    for (const auto &group : groupData)
    {
        // 检查当前分组是否在启用的 groupIds 中
        if (enabledGroupIds.contains(group.groupId))
        {
            // 添加到 groups 和 groupDescriptions 中
            groups.append(QString::number(group.groupId));
            groupDescriptions.append("\"" + group.groupDescription + "\"");

            // 获取 groupWeights 中的权重
            if (latestVersion.groupWeights.find(group.groupId) != latestVersion.groupWeights.end())
            {
                float weight = latestVersion.groupWeights.at(group.groupId);
                groupMap[group.groupId] = weight;                     // 填充到 groupMap
                groupWeights.append(QString::number(weight, 'f', 2)); // 格式化为小数
            }
        }
    }

    // 构建分组配置数据的替换内容
    QString groupConfigReplacement = "{";
    bool first = true;
    for (const auto &group : groupMap)
    {
        if (!first)
        {
            groupConfigReplacement += ", "; // 如果不是第一个元素，添加逗号
        }
        groupConfigReplacement += QString("{%1, %2}").arg(group.first).arg(group.second); // 正确的初始化格式
        first = false;
    }
    groupConfigReplacement += "}, ";

    groupConfigReplacement += "{" + groupDescriptions.join(", ") + "}\n";

    // 查找并替换模板中的分组配置数据
    QString groupConfigTemplate = "static VersionConfig s_groupConfig = {\n";
    groupConfigTemplate += "    " + groupConfigReplacement;
    groupConfigTemplate += "};";

    templateContent.replace("@分组配置数据", "//自动生成的分组配置数据\n" + groupConfigTemplate);
}

void ExportCpp::replaceGroupMapping(QString &templateContent)
{
    // 从配置中获取分组映射数据
    const auto &groupMappings = LoadConfig::getInstance().getGroupMappings();

    std::unordered_map<int, int> mappings;

    // 遍历每个映射
    for (const auto &mapping : groupMappings)
    {
        mappings[mapping.originGroupId] = mapping.mappingGroupId;
    }

    // 递归更新映射关系
    for (auto &it : mappings)
    {
        while (mappings.find(it.second) != mappings.end())
        {
            it.second = mappings[it.second];
        }
    }

    QString groupMappingReplacement;
    int i = 0;
    int totalMappings = mappings.size();

    // 遍历并生成映射规则
    for (const auto &it : mappings)
    {
        groupMappingReplacement += "    {" + QString::number(it.first) + ", " + QString::number(it.second) + "}";

        // 如果不是最后一项，添加逗号
        if (i++ < totalMappings - 1)
        {
            groupMappingReplacement += ",";
        }
        groupMappingReplacement += "\n";
    }

    // 构建最终的模板
    QString groupMappingTemplate = "static std::unordered_map<int, int> s_groupMapping = {\n";
    groupMappingTemplate += groupMappingReplacement;
    groupMappingTemplate += "};";

    templateContent.replace("@分组映射规则", "//自动生成的分组映射规则\n" + groupMappingTemplate);
}

QString ExportCpp::generateBoolFunctionSmall(const LoadConfig::FunctionData &functionData) const
{
    QString functionTemplate = "bool ABTest::boolFunctionSmall() {\n   return ";

    // 构建条件表达式的字符串
    QString conditions;
    for (const auto &group : functionData.groups)
    {
        if (!conditions.isEmpty())
        {
            conditions.append(" || ");
        }
        conditions.append(QString("m_currentGroupId == %1").arg(group.first)); // 使用 group.first 作为组 ID
    }
    if (functionData.groups.empty())
    {
        conditions = "0";
    }

    functionTemplate += conditions;

    functionTemplate += ";\n}\n";

    // 替换模版中的函数名
    functionTemplate.replace("boolFunctionSmall", functionData.functionName);

    return functionTemplate;
}

QString ExportCpp::generateBoolFunctionMedium(const LoadConfig::FunctionData &functionData) const
{
    QString functionTemplate = "bool ABTest::boolFunctionMedium() {\n    static const std::unordered_set<int> groups = {";

    // 构建 groups 初始化列表
    QString groupsList;
    for (const auto &group : functionData.groups)
    {
        if (!groupsList.isEmpty())
        {
            groupsList.append(", ");
        }
        groupsList.append(QString::number(group.first)); // 使用 group.first 作为组 ID
    }

    functionTemplate += groupsList;
    functionTemplate += "};\n";
    functionTemplate += "    return groups.count(getCurrentGroup()) > 0;\n}\n";

    // 替换模版中的函数名
    functionTemplate.replace("boolFunctionMedium", functionData.functionName);

    return functionTemplate;
}

QString ExportCpp::generateBoolFunctionLarge(const LoadConfig::FunctionData &functionData) const
{
    QString functionTemplate = "bool ABTest::boolFunctionLarge() {\n";
    functionTemplate += "    static const std::vector<bool> enabled = {\n        ";
    // 获取组数据的大小
    int size = LoadConfig::getInstance().getGroupData().size();

    // 构建 enabled 数组，初始值为全 false
    QStringList enabledValues;
    for (int i = 0; i < size; ++i)
    {
        enabledValues.append("false");
    }

    // 根据 functionData.groups 中的组 ID，修改 enabled 数组
    for (const auto &group : functionData.groups)
    {
        if (group.first <= size)
        {
            enabledValues[group.first - 1] = "true"; // 将对应位置设为 true
        }
    }

    // 赋值 enabled 数组
    QString enabledList;
    int count = 0;
    for (const QString &value : enabledValues)
    {
        if (count > 0 && count % 10 == 0)
        {
            enabledList.append("\n        "); // 每十个元素换行
        }
        enabledList.append(value);
        if (count < enabledValues.size() - 1)
        {
            enabledList.append(", ");
        }
        ++count;
    }

    functionTemplate += enabledList;
    functionTemplate += "\n    };\n";
    functionTemplate += "    return m_currentGroupId - 1 < enabled.size() && enabled[m_currentGroupId - 1];\n}\n";

    // 替换模版中的函数名
    functionTemplate.replace("boolFunctionLarge", functionData.functionName);

    return functionTemplate;
}

QString ExportCpp::generateIntFunction(const LoadConfig::FunctionData &functionData) const
{
    QString functionTemplate = "int ABTest::intFunction() {\n";
    functionTemplate += "    static const std::vector<int> values = {\n        ";
    // 初始化一个与 functionData.groups 大小相同的 vector 容器
    std::vector<int> values(functionData.groups.size(), 0); // 初始值为0

    // 遍历 functionData.groups，根据键值设置对应的值
    for (const auto &group : functionData.groups)
    {
        int index = group.first;
        if (index > 0 && index <= values.size())
        {
            // 使用 at() 访问 keyValuePairs，避免 const 错误
            values[index - 1] = functionData.keyValuePairs.at(group.second).second.toInt();
        }
    }

    // 构建 values 数组的初始化列表
    QStringList valuesList;
    for (const int &value : values)
    {
        valuesList.append(QString::number(value));
    }

    // 赋值 values 数组，每10个元素换行一次
    QString valuesString;
    int count = 0;
    for (const QString &value : valuesList)
    {
        if (count > 0 && count % 10 == 0)
        {
            valuesString.append("\n        ");
        }
        valuesString.append(value);
        if (count < valuesList.size() - 1)
        {
            valuesString.append(", ");
        }
        ++count;
    }

    functionTemplate += valuesString;
    functionTemplate += "\n    };\n";
    functionTemplate += "    return m_currentGroupId - 1 < values.size() ? values[m_currentGroupId - 1] : 0;\n}\n";

    // 替换模版中的函数名
    functionTemplate.replace("intFunction", functionData.functionName);

    return functionTemplate;
}

QString ExportCpp::generateFloatFunction(const LoadConfig::FunctionData &functionData) const
{
    QString functionTemplate = "float ABTest::floatFunction() {\n";
    functionTemplate += "    static const std::vector<float> values = {\n        ";

    // 初始化一个与 functionData.groups 大小相同的 vector 容器
    std::vector<float> values(functionData.groups.size(), 0.0f); // 初始值为0.0f

    // 遍历 functionData.groups，根据键值设置对应的值
    for (const auto &group : functionData.groups)
    {
        int index = group.first;
        if (index > 0 && index <= values.size())
        {
            // 使用 at() 访问 keyValuePairs，避免 const 错误
            values[index - 1] = functionData.keyValuePairs.at(group.second).second.toFloat();
        }
    }

    // 构建 values 数组的初始化列表
    QStringList valuesList;
    for (const float &value : values)
    {
        valuesList.append(QString::number(value));
    }

    // 赋值 values 数组，每10个元素换行一次
    QString valuesString;
    int count = 0;
    for (const QString &value : valuesList)
    {
        if (count > 0 && count % 10 == 0)
        {
            valuesString.append("\n        ");
        }
        valuesString.append(value);
        if (count < valuesList.size() - 1)
        {
            valuesString.append(", ");
        }
        ++count;
    }

    functionTemplate += valuesString;
    functionTemplate += "\n    };\n";
    functionTemplate += "    return m_currentGroupId - 1 < values.size() ? values[m_currentGroupId - 1] : 0;\n}\n";

    // 替换模版中的函数名
    functionTemplate.replace("floatFunction", functionData.functionName);

    return functionTemplate;
}

QString ExportCpp::generateStringFunction(const LoadConfig::FunctionData &functionData) const
{
    QString functionTemplate = "std::string ABTest::stringFunction() {\n";
    functionTemplate += "    static const std::vector<string> values = {\n        ";

    // 初始化一个与 functionData.groups 大小相同的 vector 容器
    std::vector<std::string> values(functionData.groups.size(), "0"); // 初始值为"0"

    // 遍历 functionData.groups，根据键值设置对应的值
    for (const auto &group : functionData.groups)
    {
        int index = group.first;
        if (index > 0 && index <= values.size())
        {
            // 使用 at() 访问 keyValuePairs，避免 const 错误
            values[index - 1] = functionData.keyValuePairs.at(group.second).second.toStdString();
        }
    }

    // 构建 values 数组的初始化列表
    QStringList valuesList;
    for (const std::string &value : values)
    {
        valuesList.append(QString("\"%1\"").arg(QString::fromStdString(value))); // 添加双引号
    }

    // 赋值 values 数组，每10个元素换行一次
    QString valuesString;
    int count = 0;
    for (const QString &value : valuesList)
    {
        if (count > 0 && count % 10 == 0)
        {
            valuesString.append("\n        ");
        }
        valuesString.append(value);
        if (count < valuesList.size() - 1)
        {
            valuesString.append(", ");
        }
        ++count;
    }

    functionTemplate += valuesString;
    functionTemplate += "\n    };\n";
    functionTemplate += "    return m_currentGroupId - 1 < values.size() ? values[m_currentGroupId - 1] : \"0\";\n}\n";

    // 替换模版中的函数名
    functionTemplate.replace("stringFunction", functionData.functionName);

    return functionTemplate;
}

QString ExportCpp::checkMutex()
{
    // 行：分组描述
    // 列：互斥开关描述
    // 单元格：该分组中使用了该互斥开关且处于开启状态的功能的功能描述数组
    std::map<QString, std::map<QString, std::vector<QString>>> conflictInfo;
    std::unordered_map<QString, LoadConfig::FunctionData> functions; // 以functionName为索引
    std::unordered_map<int, QString> groups;                         // 以groupId为索引

    const auto &functionDataList = LoadConfig::getInstance().getFunctionData();
    const auto &groupDataList = LoadConfig::getInstance().getGroupData();
    const auto &mutexSwitches = LoadConfig::getInstance().getMutexSwitchData();

    for (const auto &function : functionDataList)
    {
        functions[function.functionName] = function;
    }

    for (const auto &group : groupDataList)
    {
        groups[group.groupId] = group.groupDescription;
    }

    // 遍历所有的互斥开关
    for (const auto &mutexSwitch : mutexSwitches)
    {
        // 遍历所有使用该互斥开关的功能
        for (const auto &function : mutexSwitch.functions)
        {
            // 遍历每一组
            for (const auto &group : functions[function].groups)
            {
                // 如果该组启用了该功能
                if (group.second == 1)
                {
                    QString groupDescription = groups[group.first];
                    QString mutexSwitchDescription = mutexSwitch.description;
                    QString functionDescription = functions[function].functionDescription;

                    conflictInfo[groupDescription][mutexSwitchDescription].push_back(functionDescription);
                }
            }
        }
    }

    // 存储所有冲突的警告信息
    QString warningMessage;

    // 遍历 conflictInfo，检查每个分组和互斥开关的组合是否有冲突
    for (const auto &[groupDescription, mutexSwitchMap] : conflictInfo)
    {
        for (const auto &[mutexSwitchDescription, functions] : mutexSwitchMap)
        {
            if (functions.size() > 1)
            { // 如果有多个功能使用同一个互斥开关且都处于默认开启状态
                warningMessage += QString("警告：分组 '%1' 中互斥开关 '%2' 存在多个功能同时处于开启状态：\n").arg(groupDescription).arg(mutexSwitchDescription);
                for (const QString &functionName : functions)
                {
                    warningMessage += QString("  - 功能: %1\n").arg(functionName);
                }
                warningMessage += "\n";
            }
        }
    }

    return warningMessage;
}
