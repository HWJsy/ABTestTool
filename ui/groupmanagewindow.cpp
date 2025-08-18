#include "groupmanagewindow.h"
#include "ui_groupmanagewindow.h"
#include "groupdatamanager.h"
#include "functiondatamanager.h"
#include <QStandardItemModel>
#include <QCheckBox>
#include <QInputDialog>
#include "toplevelmanager.h"
#include <QSettings>
#include <QComboBox>
GroupManageWindow::GroupManageWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GroupManageWindow)
{
    ui->setupUi(this);
    setWindowTitle("分组管理模块");
    ui->tableViewGroupList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->tableViewGroupList, &QTableView::doubleClicked, this, &GroupManageWindow::onCellDoubleClicked);
    ui->pushButtonCreateGroup->installEventFilter(this);
    ui->tableViewGroupList->installEventFilter(this);
    refresh();
}

GroupManageWindow::~GroupManageWindow()
{
    delete ui;
}

void GroupManageWindow::on_pushButtonCreateGroup_clicked()
{
    QSettings settings("App");
    QString savedPath = settings.value("configFilePath").toString();

    // 判断路径是否为空
    if (savedPath.isEmpty()) {
        // 路径为空，提示用户设置路径
        QMessageBox::warning(this, "路径未设置", "请先设置配置文件路径！");
        return;  // 不继续执行，等待用户设置路径
    }
    // 创建 newGroupWindow 对象
    newGroupWindow *groupWindow = new newGroupWindow(this);

    // 显示对话框
    groupWindow->exec();

    refresh();
}

bool GroupManageWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        qDebug() << "refresh";
        // 当控件获得焦点时，触发 refresh
        refresh();
    }
    return QDialog::eventFilter(watched, event); // 返回给父类处理
}

void GroupManageWindow::refresh()
{
    // 清空当前表格的模型
    QStandardItemModel *model = new QStandardItemModel();
    ui->tableViewGroupList->setModel(model);  // 重新设置模型

    TopLevelManager::getInstance().loadData();

    // 获取所有功能名称
    QList<FunctionData> functionDataList = FunctionDataManager::getInstance().getAllFunctionData();

    // 获取所有分组
    QList<GroupManageData> groupDataList = GroupDataManager::getInstance().getAllGroupData();

    // 设置模型的列数和行数
    model->setColumnCount(groupDataList.size() + 2);  // 第一列显示功能名，第二列显示返回类型，后续列显示分组
    model->setRowCount(functionDataList.size() + 1);  // 第一行用于显示组描述

    // 设置列头
    model->setHorizontalHeaderItem(0, new QStandardItem("功能名"));
    model->setHorizontalHeaderItem(1, new QStandardItem("返回类型"));
    for (int i = 0; i < groupDataList.size(); ++i) {
        model->setHorizontalHeaderItem(i + 2, new QStandardItem(QString::number(groupDataList[i].groupId) + "组"));
    }

    // 设置第一列显示功能名称
    for (int i = 0; i < functionDataList.size(); ++i) {
        model->setItem(i + 1, 0, new QStandardItem(functionDataList[i].functionDescription));  // 设置功能名称
    }

    // 设置第二列显示返回类型
    for (int i = 0; i < functionDataList.size(); ++i) {
        model->setItem(i + 1, 1, new QStandardItem(functionDataList[i].returnType));  // 设置返回类型
    }

    // 设置第一行显示组描述
    for (int i = 0; i < groupDataList.size(); ++i) {
        model->setItem(0, i + 2, new QStandardItem(groupDataList[i].groupDescription));
    }

    // 遍历所有功能和分组，设置每个单元格的内容
    for (int i = 0; i < functionDataList.size(); ++i) {
        for (int j = 0; j < groupDataList.size(); ++j) {
            QString functionName = functionDataList[i].functionName;
            QString returnType = functionDataList[i].returnType;

            // 如果返回类型是 bool，则显示 "✓"，否则显示返回值
            if (returnType == "bool") {
                QString returnValue = (groupDataList[j].functions[functionName] == 1) ? "✓" : " ";
                model->setItem(i + 1, j + 2, new QStandardItem(returnValue));
            } else {
                // 如果不是 bool 类型，则显示返回值
                int& id = groupDataList[j].functions[functionName]; // 选择的键值对id
                model->setItem(i + 1, j + 2, new QStandardItem(functionDataList[i].keyValuePairs[id].first));
            }
        }
    }

    // 隐藏行号
    ui->tableViewGroupList->verticalHeader()->setVisible(false);

    // 设置新的模型到视图
    ui->tableViewGroupList->setModel(model);
}

void GroupManageWindow::onCellDoubleClicked(const QModelIndex &index)
{
    // 获取当前分组和功能列表
    QList<GroupManageData> groupDataList = GroupDataManager::getInstance().getAllGroupData();
    QList<FunctionData> functionDataList = FunctionDataManager::getInstance().getAllFunctionData();

    if (index.row() > 0 && index.column() > 1) { // 排除列头和行头
        int groupIndex = index.column() - 2;
        int functionIndex = index.row() - 1;

        QMessageBox::StandardButton reply;

        if(functionDataList[functionIndex].returnType == "bool") {
            int isFunctionEnabled = groupDataList[groupIndex].functions[functionDataList[functionIndex].functionName];

            // 如果将要从关闭转为开启，检查是否存在互斥开关冲突
            if(!isFunctionEnabled && checkMutex(groupDataList[groupIndex].groupId, functionDataList[functionIndex].functionName, functionDataList[functionIndex].functionDescription)) {
                return;
            }

            // 弹出提示框询问用户
            QString message = QString("提示：%1组的 %2 功能 由 %3 转为 %4 ")
                                  .arg(groupDataList[groupIndex].groupId)
                                  .arg(functionDataList[functionIndex].functionDescription)
                                  .arg(isFunctionEnabled == 1 ? "开启" : "关闭")
                                  .arg(isFunctionEnabled == 1 ? "关闭" : "开启");
            reply = QMessageBox::question(this, "修改功能状态", message,
                                          QMessageBox::Yes | QMessageBox::No);
            // 如果用户确认修改
            if (reply == QMessageBox::Yes) {
                // 更新功能状态
                GroupDataManager::getInstance().updateGroupFunction(groupDataList[groupIndex].groupId, functionDataList[functionIndex].functionName);
            }
        } else {
            // 创建弹窗修改返回值
            QDialog *dialog = new QDialog(this);
            dialog->setWindowTitle("修改返回值");

            QLabel *label = new QLabel("当前返回值类型为：" + functionDataList[functionIndex].returnType, dialog);
            QLabel *label1 = new QLabel("请选择新的返回值:", dialog);
            QComboBox *comboBox = new QComboBox(dialog);
            QPushButton *restoreButton = new QPushButton("恢复默认值", dialog);
            QPushButton *confirmButton = new QPushButton("确定", dialog);
            QPushButton *cancelButton = new QPushButton("取消", dialog);

            QVBoxLayout *layout = new QVBoxLayout(dialog);
            layout->addWidget(label);
            layout->addWidget(label1);
            layout->addWidget(comboBox);
            layout->addWidget(restoreButton);
            layout->addWidget(confirmButton);
            layout->addWidget(cancelButton);

            // for(auto it: functionDataList[functionIndex].keyValuePairs)
            // {
            //     comboBox->addItem(it.first);
            // }
            for(int id = 0;id < functionDataList[functionIndex].keyValuePairs.size();id++)
            {
                comboBox->addItem(functionDataList[functionIndex].keyValuePairs[id].first);
            }

            comboBox->setCurrentIndex(groupDataList[groupIndex].functions[functionDataList[functionIndex].functionName]);

            // 恢复默认值
            connect(restoreButton, &QPushButton::clicked, dialog, [dialog, groupDataList, groupIndex, functionDataList, functionIndex]() {
                GroupDataManager::getInstance().updateGroupFunction(
                    groupDataList[groupIndex].groupId,
                    functionDataList[functionIndex].functionName,
                    functionDataList[functionIndex].currentKeyValueId);
                dialog->accept();  // 关闭对话框
            });

            // 确定按钮：检查输入并提交
            connect(confirmButton, &QPushButton::clicked, dialog, [this, &groupDataList, &functionDataList, groupIndex, functionIndex, comboBox, dialog]() {
                int newReturnId = comboBox->currentIndex();

                GroupDataManager::getInstance().updateGroupFunction(
                    groupDataList[groupIndex].groupId,
                    functionDataList[functionIndex].functionName,
                    newReturnId);

                dialog->accept();  // 关闭对话框
            });

            // 取消按钮：关闭对话框
            connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

            // 显示弹窗
            dialog->exec();
        }

        refresh();
    } else if (index.row() == 0 && index.column() > 1) { // 分组描述修改
        int groupIndex = index.column() - 2;

        // 弹出输入框，接收用户输入的新分组描述
        bool ok;
        QString newDescription = QInputDialog::getText(this, "修改分组描述", "请输入新的分组描述：", QLineEdit::Normal, groupDataList[groupIndex].groupDescription, &ok);

        // 检查用户是否输入了描述
        if (ok && !newDescription.isEmpty()) {
            // 调用 GroupDataManager 的更新函数来修改分组描述
            GroupDataManager::getInstance().updateGroupDescription(groupDataList[groupIndex].groupId, newDescription);

            // 刷新表格
            refresh();
        } else if (ok && newDescription.isEmpty()) {
            // 如果用户没有输入描述，显示警告
            QMessageBox::warning(this, "输入错误", "分组描述不能为空！");
        }

    }
}

bool GroupManageWindow::checkMutex(int groupId, QString functionName, QString functionDescription)
{
    // 检查functionName对应的function是否有互斥锁
    const auto& mutexSwitches = FunctionDataManager::getInstance().getMutexSwitches();
    MutexSwitchData mutexSwitch;
    bool isFound = false;

    for(const auto& it: mutexSwitches) {
        if(it.functions.find(functionName) != it.functions.end()) {
            isFound = true;
            mutexSwitch = it;
            break;
        }
    }

    // 如果没有设置互斥锁，那么返回false
    if(!isFound) {
        return false;
    }

    // 如果互斥锁中functions大小为1，则直接返回false
    if(mutexSwitch.functions.size() == 1) {
        return false;
    }
    const auto& group = GroupDataManager::getInstance().getGroupList()[groupId-1];
    qDebug() << group.groupDescription;

    // 检查当前组中是否有使用了同一个互斥开关且处于开启状态的功能
    for(const auto& function: mutexSwitch.functions) {
        if(group.functions.at(function) == 1 && function != functionName) {
            auto conflictFunctionDescription = FunctionDataManager::getInstance().getFunctionByName(function).functionDescription;
            // 弹出提示窗口
            QString warningMessage = QString("警告：无法修改功能 '%1' 的状态！\n\n当前分组中，功能 '%2'使用了相同的互斥开关并且处于开启状态。")
                                         .arg(functionDescription)
                                         .arg(conflictFunctionDescription);

            QMessageBox::warning(this, "互斥开关冲突", warningMessage);
            return true;
        }
    }
    return false;
}
