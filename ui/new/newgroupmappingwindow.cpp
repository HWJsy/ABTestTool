#include "newgroupmappingwindow.h"
#include "ui_newgroupmappingwindow.h"
#include <QMessageBox>
#include <QDebug>
#include "groupdatamanager.h"
#include "groupmappingdatamanager.h"

NewGroupMappingWindow::NewGroupMappingWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewGroupMappingWindow)
{
    ui->setupUi(this);

    // 初始化 combobox 中的内容
    QList<GroupManageData> groupDataList = GroupDataManager::getInstance().getAllGroupData();
    for (const GroupManageData &groupData : groupDataList) {
        QString displayText = QString("%1 - %2").arg(groupData.groupId).arg(groupData.groupDescription);
        ui->comboBoxOrigin->addItem(displayText, groupData.groupId);
        ui->comboBoxMapping->addItem(displayText, groupData.groupId);
    }
}


NewGroupMappingWindow::~NewGroupMappingWindow()
{
    delete ui;
}

void NewGroupMappingWindow::on_buttonBox_accepted()
{
    // 获取用户选择的 ID
    int originGroupId = ui->comboBoxOrigin->currentData().toInt();  // 获取 origin comboBox 的数据
    int mappingGroupId = ui->comboBoxMapping->currentData().toInt();  // 获取 mapping comboBox 的数据

    // 检查 ID 是否存在于 GroupDataManager 中
    bool originIdExists = false;
    bool mappingIdExists = false;
    for (const GroupManageData &groupData : GroupDataManager::getInstance().getAllGroupData()) {
        if (groupData.groupId == originGroupId) {
            originIdExists = true;
        }
        if (groupData.groupId == mappingGroupId) {
            mappingIdExists = true;
        }
        if (originIdExists && mappingIdExists) {
            break;
        }
    }

    // 检查 ID 是否已经在 GroupMappingDataManager 中
    auto &groupMapping = GroupMappingDataManager::getInstance().getGroupMapping();
    if (groupMapping.find(originGroupId) != groupMapping.end()) {
        QMessageBox::warning(this, "ID 已存在", "该原始分组ID已经存在于映射中！");
        return;
    }

    if (mappingGroupId == originGroupId) {
        QMessageBox::warning(this, "自我映射", "不可自己映射到自己！");
        return;
    }

    // 创建一个临时副本，避免修改原始的映射表
    std::unordered_map<int, int> tempGroupMapping = groupMapping;

    // 将待新建的映射关系加入副本
    tempGroupMapping[originGroupId] = mappingGroupId;

    // 检查是否会导致环形映射
    int currentGroupId = mappingGroupId;
    while (tempGroupMapping.find(currentGroupId) != tempGroupMapping.end()) {
        currentGroupId = tempGroupMapping[currentGroupId];  // 获取下一个映射的分组ID

        // 如果当前分组ID等于原始分组ID，则说明发生了循环映射
        if (currentGroupId == originGroupId) {
            QMessageBox::warning(this, "环形映射", "该映射会导致循环映射，无法添加！");
            return;
        }
    }

    // 如果一切都通过，添加映射关系
    GroupMappingDataManager::getInstance().addMapping(originGroupId, mappingGroupId);

    // 关闭对话框
    accept();
}

void NewGroupMappingWindow::on_buttonBox_rejected()
{
    reject();
}
