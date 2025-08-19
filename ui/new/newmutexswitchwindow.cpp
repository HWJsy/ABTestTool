#include "newmutexswitchwindow.h"
#include "ui_newmutexswitchwindow.h"
#include "functiondatamanager.h"
#include "toplevelmanager.h"
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>

NewMutexSwitchWindow::NewMutexSwitchWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewMutexSwitchWindow)
{
    ui->setupUi(this);
    QStandardItemModel *model = new QStandardItemModel();
    ui->tableViewMutexSwitch->setModel(model);
    refresh();
    ui->tableViewMutexSwitch->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->tableViewMutexSwitch, &QTableView::doubleClicked, this, &NewMutexSwitchWindow::onCellDoubleClicked);
}

NewMutexSwitchWindow::~NewMutexSwitchWindow()
{
    delete ui;
}

void NewMutexSwitchWindow::on_pushButtonCreateMutexSwitch_clicked()
{
    // 创建新的弹窗（QDialog）
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("添加互斥开关");

    // 创建 QLineEdit 让用户输入互斥开关名称
    QLineEdit *lineEditMutexSwitchName = new QLineEdit(dialog);
    lineEditMutexSwitchName->setPlaceholderText("请输入互斥开关的名称");

    // 创建“确定”按钮
    QPushButton *pushButtonConfirm = new QPushButton("确定", dialog);

    // 连接“确定”按钮点击信号与槽函数
    connect(pushButtonConfirm, &QPushButton::clicked, [this, dialog, lineEditMutexSwitchName]() {
        QString newSwitchName = lineEditMutexSwitchName->text().trimmed();

        // 检查用户是否输入了名称
        if (newSwitchName.isEmpty()) {
            // 如果没有输入名称，弹出提示框
            QMessageBox::warning(dialog, "输入错误", "请输入互斥开关的名称！");
            return;
        }

        // 检查是否已经有重复的名称
        QList<MutexSwitchData>& mutexSwitches = FunctionDataManager::getInstance().getMutexSwitches();
        for (const MutexSwitchData& mutexSwitch : mutexSwitches) {
            if (mutexSwitch.description == newSwitchName) {
                // 如果找到重复的名称，弹出警告框
                QMessageBox::warning(dialog, "名称重复", "该名称已存在，请输入一个不同的名称！");
                return;
            }
        }

        // 调用 FunctionDataManager 的实例，添加新互斥开关
        FunctionDataManager::getInstance().addMutexSwitch(newSwitchName);

        // 关闭弹窗
        dialog->accept();
        refresh();
    });

    // 设置布局
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(lineEditMutexSwitchName);
    layout->addWidget(pushButtonConfirm);
    dialog->setLayout(layout);

    // 执行弹窗
    dialog->exec();
}



// 刷新tableview视图
void NewMutexSwitchWindow::refresh()
{
    // 获取所有的互斥开关数据
    QList<MutexSwitchData>& mutexSwitches = FunctionDataManager::getInstance().getMutexSwitches();

    // 清空现有的模型数据
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableViewMutexSwitch->model());
    model->setColumnCount(1);  // 只有一列
    model->setHorizontalHeaderLabels({"互斥开关"});  // 设置列头

    // 向模型中添加每个互斥开关的描述
    for (const MutexSwitchData &mutexSwitch : mutexSwitches) {
        // 将互斥开关的描述作为一行数据
        QStandardItem *item = new QStandardItem(mutexSwitch.description);
        model->appendRow(item);
    }
    ui->tableViewMutexSwitch->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void NewMutexSwitchWindow::onCellDoubleClicked(const QModelIndex &index)
{
    // 获取被双击的行的 id
    int row = index.row();
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableViewMutexSwitch->model());
    QString currentSwitchName = model->data(model->index(row, 0)).toString();  // 获取互斥开关的名称

    // 创建弹窗
    QDialog *editDialog = new QDialog(this);
    editDialog->setWindowTitle("编辑互斥开关名称");

    // 创建 QLineEdit，默认填入当前名称
    QLineEdit *lineEdit = new QLineEdit(currentSwitchName, editDialog);
    QPushButton *pushButtonConfirm = new QPushButton("确定", editDialog);

    // 确认按钮的点击事件
    connect(pushButtonConfirm, &QPushButton::clicked, [this, row, lineEdit, editDialog]() {
        QString newSwitchName = lineEdit->text().trimmed();

        // 如果名称为空，弹出警告框
        if (newSwitchName.isEmpty()) {
            QMessageBox::warning(editDialog, "输入错误", "请输入互斥开关的名称！");
            return;
        }

        // 检查是否有重复的名称（排除当前正在编辑的行）
        QList<MutexSwitchData>& mutexSwitches = FunctionDataManager::getInstance().getMutexSwitches();
        for (int i = 0; i < mutexSwitches.size(); ++i) {
            // 跳过当前正在编辑的行
            if (i != row && mutexSwitches[i].description == newSwitchName) {
                // 如果找到重复的名称，弹出警告框
                QMessageBox::warning(editDialog, "名称重复", "该名称已存在，请输入一个不同的名称！");
                return;
            }
        }

        // 更新互斥开关名称
        mutexSwitches[row].description = newSwitchName;

        // 刷新视图
        refresh();

        TopLevelManager::getInstance().saveData();

        // 关闭编辑对话框
        editDialog->accept();
    });

    // 设置弹窗布局
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(lineEdit);
    layout->addWidget(pushButtonConfirm);
    editDialog->setLayout(layout);

    // 显示弹窗
    editDialog->exec();
}

