#include "newgroupwindow.h"
#include "ui_newgroupwindow.h"
#include "groupdatamanager.h"
#include "functiondatamanager.h"
#include <QDebug>
newGroupWindow::newGroupWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::newGroupWindow)
{
    ui->setupUi(this);
    lineEditGroupDescription = ui->lineEditGroupDescription;
    buttonBox = ui->buttonBox;
}

newGroupWindow::~newGroupWindow()
{
    delete ui;
}

void newGroupWindow::on_buttonBox_accepted() {
    if(checkMutex()) {
        return;
    }

    QString groupDescription = lineEditGroupDescription->text();

    // 如果用户没有输入分组描述，弹出提示框
    if (groupDescription.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "分组描述不能为空！");
        return;
    }
    GroupDataManager::getInstance().addGroup(groupDescription);
    accept();
}

// 检查是否有功能使用了同一个功能，且都处于默认开启状态
bool newGroupWindow::checkMutex()
{
    std::map<QString, std::vector<QString>> warningInfo;  //<互斥开关名称, <使用该互斥开关且处于默认开启状态的功能名>>

    const auto& mutexSwitches = FunctionDataManager::getInstance().getMutexSwitches();

    // 遍历所有互斥开关
    for (const MutexSwitchData& mutexSwitch : mutexSwitches) {
        // 遍历其中的每一个功能
        for (const auto& function : mutexSwitch.functions) {
            const auto& functionData = FunctionDataManager::getInstance().getFunctionByName(function);

            // 如果该功能处于默认开启状态（currentKeyValueId == 1）
            if (functionData.currentKeyValueId == 1) {
                warningInfo[mutexSwitch.description].push_back(function);
            }
        }
    }

    // 存储所有冲突的警告信息
    QString warningMessage;

    // 检查 warningInfo 是否存在冲突（即某个互斥开关下有多个功能同时处于开启状态）
    for (const auto& [mutexSwitchDescription, functions] : warningInfo) {
        // 如果某个互斥开关下的功能数大于1，表示存在冲突
        if (functions.size() > 1) {
            warningMessage += QString("互斥开关 '%1' 存在多个功能同时处于默认开启状态：\n").arg(mutexSwitchDescription);

            // 将所有冲突的功能加入警告信息中
            for (const QString& functionName : functions) {
                QString functionDescription = FunctionDataManager::getInstance().getFunctionByName(functionName).functionDescription;
                warningMessage += QString("  - %1\n").arg(functionDescription);
            }
            warningMessage += "\n";
        }
    }

    // 如果有冲突，弹出提示框并返回 true
    if (!warningMessage.isEmpty()) {
        warningMessage += "无法添加新的分组\n";
        QMessageBox::warning(this, "互斥开关冲突", warningMessage);
        return true;
    }

    return false;
}

void newGroupWindow::on_buttonBox_rejected() {
    reject();  // 关闭对话框
}
