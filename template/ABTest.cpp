//
//  ABTestNewDemo.cpp
//  SuperFootball
//

#include "ABTest.hpp"
#include <unordered_set>
#include "cocos2d.h"

// 静态成员
static ABTest* s_instance = nullptr;

// 配置常量
#define KEY_GROUP_ID "abtest_now_group_id"

#define KEY_FIRST_GROUP_ID "abtest_first_group_id"

@分组配置数据

@分组映射规则

ABTest::ABTest() 
    : m_currentGroupId(-1) {
    // 构造函数
}

ABTest::~ABTest() {
    // 析构函数
}

ABTest* ABTest::getInstance() {
    if (s_instance == nullptr) {
        s_instance = new ABTest();
        s_instance->initialize(); // 确保在第一次获取实例时进行初始化
    }
    return s_instance;
}

void ABTest::initialize() {
    // 初始化用户分组
    initializeUserGroup();
}

bool ABTest::initializeUserGroup() {
    // 从本地加载分组记录到成员变量
    m_currentGroupId = cocos2d::UserDefault::getInstance()->getIntegerForKey(KEY_GROUP_ID, -1);
    
    if (m_currentGroupId != -1) {
        // 检查是否需要更新分组
        if (checkAndUpdateGroupId()) {
            CCLOG("[ABTest] Group updated to: %d", m_currentGroupId);
        }
        
        return true;
    }
    
    if (s_groupConfig.groups.empty()) {
        CCAssert(false, "[ABTest] ERROR: Group config is empty");
        return false;
    }
    
    // 基于权重的分组分配
    double randomValue = cocos2d::random<float>(0.0, 1.0);
    
    // 根据权重选择分组
    double cumulativeWeight = 0.0;
    int assignedGroupId = s_groupConfig.groups.begin()->first; // 默认值
    
    for (const auto& pair : s_groupConfig.groups) {
        cumulativeWeight += pair.second;
        if (randomValue <= cumulativeWeight) {
            assignedGroupId = pair.first;
            break;
        }
    }
    
    // 更新成员变量并保存到本地
    m_currentGroupId = assignedGroupId;
    cocos2d::UserDefault::getInstance()->setIntegerForKey(KEY_GROUP_ID, m_currentGroupId);
    cocos2d::UserDefault::getInstance()->setIntegerForKey(KEY_FIRST_GROUP_ID, m_currentGroupId);

    CCLOG("[ABTest] New user assigned to group: %d", assignedGroupId);
    return true;
}

int ABTest::getInitialGroup(){
    return cocos2d::UserDefault::getInstance()->getIntegerForKey(KEY_FIRST_GROUP_ID, -1);
}

bool ABTest::checkAndUpdateGroupId() {

    bool hasAnyUpdate = false;
    // 循环检查分组映射，直到无法继续更新

        // 检查是否有分组映射规则
        auto mappingIter = s_groupMapping.find(m_currentGroupId);
        if (mappingIter != s_groupMapping.end()) {
        m_currentGroupId = mappingIter->second;
        // 保存最终的分组ID到本地
        cocos2d::UserDefault::getInstance()->setIntegerForKey(KEY_GROUP_ID, m_currentGroupId);
            // 更新成员变量
            hasAnyUpdate = true;
    }
    
    return hasAnyUpdate;
}

int ABTest::getCurrentGroup() {
    return m_currentGroupId;
}

//给debug使用
// 允许在调试时手动设置当前分组ID
void ABTest::setCurrentGroupForDebug(int groupId) {
        m_currentGroupId = groupId;
 }

std::string ABTest::getGroupName(int groupId) {
    // 在分组配置中查找分组名称
    int index = 0;
    for (const auto& pair : s_groupConfig.groups) {
        if (pair.first == groupId && index < static_cast<int>(s_groupConfig.groupDescs.size())) {
            return s_groupConfig.groupDescs[index];
        }
        index++;
    }
    return "Unknown Group";
}

std::vector<std::string> ABTest::getAllGroupNames() {
    // 返回所有分组名称
    return s_groupConfig.groupDescs;
}

std::vector<int> ABTest::getAllGroupIds() {
    // 返回所有分组ID
    std::vector<int> groupIds;
    for (const auto& pair : s_groupConfig.groups) {
        groupIds.push_back(pair.first);
    }
    return groupIds;
}

@函数实现