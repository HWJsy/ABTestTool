#include "groupupdatemappingwindow.h"
#include "ui_groupupdatemappingwindow.h"
#include "groupmappingdatamanager.h"
#include "groupdatamanager.h"
#include "newgroupmappingwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QStandardItemModel>
#include <qDebug>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include "toplevelmanager.h"
#include <QSettings>
GroupUpdateMappingWindow::GroupUpdateMappingWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupUpdateMappingWindow)
{
    ui->setupUi(this);
    setWindowTitle("更新映射模块");

    ui->tableViewGroupMapping->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewAfterMapping->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 连接双击事件
    connect(ui->tableViewGroupMapping, &QTableView::doubleClicked, this, &GroupUpdateMappingWindow::onCellDoubleClicked);
    ui->tableViewGroupMapping->installEventFilter(this);
    ui->tableViewAfterMapping->installEventFilter(this);
    // 初始化表格内容
    refresh();
}

GroupUpdateMappingWindow::~GroupUpdateMappingWindow()
{
    delete ui;
}

void GroupUpdateMappingWindow::on_pushButtonCreateMapping_clicked()
{
    QSettings settings("App");
    QString savedPath = settings.value("configFilePath").toString();

    // 判断路径是否为空
    if (savedPath.isEmpty()) {
        // 路径为空，提示用户设置路径
        QMessageBox::warning(this, "路径未设置", "请先设置配置文件路径！");
        return;  // 不继续执行，等待用户设置路径
    }
    NewGroupMappingWindow *groupMappingWindow = new NewGroupMappingWindow(this);

    // 显示对话框
    groupMappingWindow->exec();

    refresh();
}

bool GroupUpdateMappingWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        refresh();
    }
    return QDialog::eventFilter(watched, event); // 返回给父类处理
}

void GroupUpdateMappingWindow::refresh()
{
    // 清空当前表格的模型
    QStandardItemModel *model = new QStandardItemModel();
    ui->tableViewGroupMapping->setModel(model);  // 重新设置模型

    TopLevelManager::getInstance().loadData();

    // 获取所有映射数据
    auto& groupMapping = GroupMappingDataManager::getInstance().getGroupMapping();

    // 设置表格的列数和行数
    model->setColumnCount(2);
    model->setRowCount(groupMapping.size());

    // 设置表格头
    model->setHorizontalHeaderItem(0, new QStandardItem("原始分组"));
    model->setHorizontalHeaderItem(1, new QStandardItem("更新后分组"));
    QList<GroupManageData> groupDataList = GroupDataManager::getInstance().getAllGroupData();
    // 填充表格数据
    int row = 0;
    for (const auto& mapping : groupMapping) {
        int originGroupId = mapping.first;
        int updatedGroupId = mapping.second;
        QString displayText1 = QString("%1 - %2").arg(originGroupId).arg(groupDataList[originGroupId - 1].groupDescription);
        QString displayText2 = QString("%1 - %2").arg(updatedGroupId).arg(groupDataList[updatedGroupId - 1].groupDescription);
        // 填充原始分组列
        model->setItem(row, 0, new QStandardItem(displayText1));  // 原始分组ID
        model->setItem(row, 1, new QStandardItem(displayText2));  // 更新后分组ID

        row++;
    }

    // 隐藏行号
    ui->tableViewGroupMapping->verticalHeader()->setVisible(false);

    ui->tableViewGroupMapping->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableViewGroupMapping->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    // 设置新的模型到视图
    ui->tableViewGroupMapping->setModel(model);
    refreshAfterMapping();
}

void GroupUpdateMappingWindow::refreshAfterMapping()
{
    // 清空当前表格的模型
    QStandardItemModel *model = new QStandardItemModel();
    ui->tableViewAfterMapping->setModel(model);  // 重新设置模型

    TopLevelManager::getInstance().loadData();

    // 获取所有映射数据
    auto& groupMapping = GroupMappingDataManager::getInstance().getGroupMapping();

    // 设置表格的列数和行数
    model->setColumnCount(2);
    model->setRowCount(groupMapping.size());

    // 设置表格头
    model->setHorizontalHeaderItem(0, new QStandardItem("原始分组"));
    model->setHorizontalHeaderItem(1, new QStandardItem("最终映射分组"));

    // 获取所有分组数据
    QList<GroupManageData> groupDataList = GroupDataManager::getInstance().getAllGroupData();

    // 填充表格数据
    int row = 0;
    for (const auto& mapping : groupMapping) {
        int originGroupId = mapping.first;
        int finalGroupId = mapping.second;

        // 通过循环获取最终的分组ID
        while (groupMapping.find(finalGroupId) != groupMapping.end()) {
            finalGroupId = groupMapping[finalGroupId];  // 获取映射的下一个分组
        }

        // 获取分组描述并格式化为"ID - 描述"
        QString displayText1 = QString("%1 - %2").arg(originGroupId).arg(groupDataList[originGroupId - 1].groupDescription);
        QString displayText2 = QString("%1 - %2").arg(finalGroupId).arg(groupDataList[finalGroupId - 1].groupDescription);

        // 填充原始分组列
        model->setItem(row, 0, new QStandardItem(displayText1));  // 原始分组ID和描述

        // 填充最终映射的分组列
        model->setItem(row, 1, new QStandardItem(displayText2));  // 最终映射的分组ID和描述

        row++;
    }

    // 隐藏行号
    ui->tableViewAfterMapping->verticalHeader()->setVisible(false);

    // 设置列宽：将两列平分宽度
    ui->tableViewAfterMapping->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableViewAfterMapping->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // 设置新的模型到视图
    ui->tableViewAfterMapping->setModel(model);
}



void GroupUpdateMappingWindow::onCellDoubleClicked(const QModelIndex &index)
{
    int row = index.row();

    // 获取映射数据
    auto& groupMapping = GroupMappingDataManager::getInstance().getGroupMapping();

    // 获取所有映射的键值对
    auto it = groupMapping.begin();
    std::advance(it, row);  // 使用迭代器找到对应的行项

    int originGroupId = it->first;   // 原始分组ID（键）
    int updatedGroupId = it->second; // 更新后分组ID（值）

    // 创建弹窗和控件
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("编辑映射关系");
    dialog->resize(400, dialog->height());

    // 创建两个下拉框分别用于选择原始分组和映射后的分组
    QComboBox *originComboBox = new QComboBox(dialog);
    QComboBox *updatedComboBox = new QComboBox(dialog);

    // 填充分组数据
    QList<GroupManageData> groupDataList = GroupDataManager::getInstance().getAllGroupData();
    for (const GroupManageData &groupData : groupDataList) {
        QString displayText = QString("%1 - %2").arg(groupData.groupId).arg(groupData.groupDescription);
        originComboBox->addItem(displayText, groupData.groupId);  // 添加原始分组下拉项
        updatedComboBox->addItem(displayText, groupData.groupId); // 添加更新后的分组下拉项
    }

    // 设置当前选择项
    originComboBox->setCurrentIndex(originComboBox->findData(originGroupId));
    updatedComboBox->setCurrentIndex(updatedComboBox->findData(updatedGroupId));

    QLabel *originLabel = new QLabel("原始分组id", dialog);
    QLabel *updatedLabel = new QLabel("映射分组id", dialog);

    QPushButton *confirmButton = new QPushButton("确定", dialog);
    QPushButton *cancelButton = new QPushButton("取消", dialog);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(originLabel);
    layout->addWidget(originComboBox);
    layout->addWidget(updatedLabel);
    layout->addWidget(updatedComboBox);
    layout->addWidget(confirmButton);
    layout->addWidget(cancelButton);

    dialog->setLayout(layout);

    // 统一的槽函数来处理映射关系逻辑
    connect(confirmButton, &QPushButton::clicked, this, [this, originComboBox, updatedComboBox, originGroupId, updatedGroupId, &groupMapping, dialog]() {
        int newOriginGroupId = originComboBox->currentData().toInt(); // 获取新原始分组ID
        int newUpdatedGroupId = updatedComboBox->currentData().toInt(); // 获取新映射分组ID

        // 校验新原始分组ID是否与其他映射重复，排除当前行的原始分组ID
        for (auto it = groupMapping.begin(); it != groupMapping.end(); ++it) {
            if (it->first != originGroupId && it->first == newOriginGroupId) {
                QMessageBox::warning(this, "ID重复", "该原始分组ID已经存在于映射关系中！");
                return;
            }
        }

        // 校验新原始分组ID列：不可映射到自己
        if (newOriginGroupId == newUpdatedGroupId) {
            QMessageBox::warning(this, "自我映射", "不可自己映射到自己！");
            return;
        }

        // 创建临时映射表
        std::unordered_map<int, int> tempGroupMapping = groupMapping;

        // 从临时映射表中删除待修改的映射
        tempGroupMapping.erase(originGroupId);

        // 将修改后的映射加入临时映射表
        tempGroupMapping[newOriginGroupId] = newUpdatedGroupId;

        // 检查是否会导致环形映射
        int currentGroupId = newUpdatedGroupId;
        while (tempGroupMapping.find(currentGroupId) != tempGroupMapping.end()) {
            currentGroupId = tempGroupMapping[currentGroupId];  // 获取下一个映射的分组ID
            if (currentGroupId == newOriginGroupId) {
                QMessageBox::warning(this, "环形映射", "该映射会导致循环映射，无法添加！");
                return;
            }
        }

        // 删除原始映射关系
        GroupMappingDataManager::getInstance().removeMapping(originGroupId);

        // 更新映射关系
        GroupMappingDataManager::getInstance().addMapping(newOriginGroupId, newUpdatedGroupId);

        // 刷新表格
        refresh();

        dialog->accept();  // 只有在所有条件通过后，才关闭对话框
    });

    // 取消按钮：关闭窗口
    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    // 显示模态对话框
    dialog->exec();
}



