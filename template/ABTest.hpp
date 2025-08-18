#ifndef ABTest_hpp
#define ABTest_hpp

#include <string>
#include <vector>
#include <map>

// 版本配置结构
struct VersionConfig
{
    std::map<int, double> groups; // key: 分组ID, value: 权重(0-1)
    std::vector<std::string> groupDescs;
};

class ABTest
{
public:
    static ABTest *getInstance();

    // 初始化系统
    void initialize();
    // 获取初始分组，给GT用，保证用户不乱跳组
    int getInitialGroup();

    // 用户分组相关
    int getCurrentGroup();

    void setCurrentGroupForDebug(int groupId);

private:
    bool initializeUserGroup();
    bool checkAndUpdateGroupId();

public:
    std::string getGroupName(int groupId);
    std::vector<std::string> getAllGroupNames();
    std::vector<int> getAllGroupIds();

private:
    ABTest();
    ~ABTest();

    // 成员变量
    int m_currentGroupId;

public:
    @函数声明
};

#endif /* ABTest_hpp */
