#include "versionmanagewindow.h"
#include "ui_versionmanagewindow.h"
#include "newversionwindow.h"
#include "versiondatamanager.h"
#include "groupdatamanager.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QInputDialog>
#include "toplevelmanager.h"
#include <QSettings>
#include <QCheckBox>
#include <QLabel>
VersionManageWindow::VersionManageWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VersionManageWindow)
{
    ui->setupUi(this);
    setWindowTitle("版本管理模块");

    ui->tableViewVersionList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->tableViewVersionList, &QTableView::doubleClicked, this, &VersionManageWindow::onCellDoubleClicked);

    QStandardItemModel *model = new QStandardItemModel();
    ui->tableViewVersionList->setModel(model);

    ui->pushButtonCreateVersion->installEventFilter(this);
    ui->tableViewVersionList->installEventFilter(this);
    totalWeightsOver1 = false;
    refresh();
}

VersionManageWindow::~VersionManageWindow()
{
    delete ui;
}

void VersionManageWindow::on_pushButtonCreateVersion_clicked()
{
    QSettings settings("App");
    QString savedPath = settings.value("configFilePath").toString();

    // 判断路径是否为空
    if (savedPath.isEmpty()) {
        // 路径为空，提示用户设置路径
        QMessageBox::warning(this, "路径未设置", "请先设置配置文件路径！");
        return;  // 不继续执行，等待用户设置路径
    }
    newVersionWindow *createVersionWindow = new newVersionWindow(this);

    createVersionWindow->exec();
    refresh();
}

bool VersionManageWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        // 当控件获得焦点时，触发 refresh
        refresh();
    }
    return QDialog::eventFilter(watched, event); // 返回给父类处理
}

void VersionManageWindow::refresh()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableViewVersionList->model());

    TopLevelManager::getInstance().loadData();

    // 获取所有分组数据
    QList<GroupManageData> groupDataList = GroupDataManager::getInstance().getAllGroupData();

    // 获取所有版本数据
    QList<VersionManageData>& versionDataList = VersionDataManager::getInstance().getVersionDataList();

    // 设置列头（版本ID作为列标题）
    model->setHorizontalHeaderItem(0, new QStandardItem("分组ID"));
    model->setHorizontalHeaderItem(1, new QStandardItem("组描述"));
    for (int i = 0; i < versionDataList.size(); ++i) {
        model->setHorizontalHeaderItem(i + 2, new QStandardItem(versionDataList[i].versionId));  // 版本ID作为列头
    }

    // “是否平均权重”行
    model->setItem(1, 0, new QStandardItem("是否平均权重"));
    // 为这一行的所有单元格添加 QCheckBox
    for (int i = colOffset; i < versionDataList.size() + colOffset; ++i) {
        // 创建 QCheckBox
        QCheckBox *checkBox = new QCheckBox();

        // 根据 version.isAverage 初始化复选框的选中状态
        checkBox->setChecked(versionDataList[i - colOffset].isAverage);

        // 将 QCheckBox 放入相应的单元格中
        ui->tableViewVersionList->setIndexWidget(model->index(1, i), checkBox);

        // 为每个复选框添加状态改变的回调
        connect(checkBox, &QCheckBox::toggled, this, [this, checkBox, i, &versionDataList](bool checked) {
            // 获取当前版本对象
            VersionManageData &version = versionDataList[i - colOffset];  // 计算当前版本的索引

            // 更新版本的 isAverage 成员
            version.isAverage = checked;
            if(checked) {
                float weight = 1.0 / versionDataList[i - colOffset].groupIds.size();
                for(const auto& groupId: versionDataList[i - colOffset].groupIds) {
                    versionDataList[i - colOffset].groupWeights[groupId] = weight;
                }
            }
            TopLevelManager::getInstance().saveData();
            this->refresh();
        });
    }

    // “权重总和”行
    model->setItem(2, 0, new QStandardItem("权重总和"));
    // 为这一行的所有单元格添加 QLabel
    for (int i = colOffset; i < versionDataList.size() + colOffset; ++i) {
        QLabel *label = new QLabel();

        // 获取当前版本对象
        VersionManageData &version = versionDataList[i - colOffset];  // 获取当前版本数据

        // 如果启用了平均权重，直接设置为100%
        if (version.isAverage) {
            label->setText("100%");
            totalWeightsOver1 = false;
        } else {
            // 否则，遍历 groupWeights 计算权重总和
            float totalWeight = 0.0f;
            for (const auto &weight : version.groupWeights) {
                totalWeight += weight.second * 100.0f;  // 将每个分组的权重加到总权重
            }
            QString totalWeightStr = QString::number(totalWeight, 'f', 2); // 格式化为两位小数
            // 去除末尾的零和小数点
            totalWeightStr = totalWeightStr.remove(QRegExp("\\.?0+$"));
            label->setText(totalWeightStr + "%");

            if (totalWeightStr != "100") {
                label->setStyleSheet("background-color: rgba(255, 0, 0, 64);");
                // 如果是最新的版本
                if(i == versionDataList.size() + colOffset - 1) {
                    totalWeightsOver1 = true;
                }
            } else {
                label->setStyleSheet("");
                if(i == versionDataList.size() + colOffset - 1) {
                    totalWeightsOver1 = false;
                }
            }
        }
        // 将 QLabel 放入相应的单元格中
        ui->tableViewVersionList->setIndexWidget(model->index(2, i), label);
    }

    // 设置第一列第三行之后显示组ID和组描述
    for (int i = 0; i < groupDataList.size(); ++i) {
        model->setItem(i + rowOffset, 0, new QStandardItem(QString::number(groupDataList[i].groupId)));  // 设置组ID
        model->setItem(i + rowOffset, 1, new QStandardItem(groupDataList[i].groupDescription));  // 设置组描述
    }

    // 设置第三行显示版本描述
    for (int i = 0; i < versionDataList.size(); ++i) {
        model->setItem(0, i + colOffset, new QStandardItem(versionDataList[i].versionDescription));  // 设置版本描述
    }

    // 遍历所有分组和版本，设置每个单元格的内容
    for (int i = 0; i < groupDataList.size(); ++i) {
        for (int j = 0; j < versionDataList.size(); ++j) {
            bool isGroupInVersion = versionDataList[j].groupIds.contains(groupDataList[i].groupId);
            // 根据是否包含该分组ID，设置单元格内容为 "✓" 或 " "
            QString status = " ";
            if(isGroupInVersion == 1) {
                status = "✓ " + QString::number(versionDataList[j].groupWeights[groupDataList[i].groupId] * 100, 'f', 2) + "%";
            }
            model->setItem(i + rowOffset, j + colOffset, new QStandardItem(status));
        }
    }

    // 隐藏行号
    ui->tableViewVersionList->verticalHeader()->setVisible(false);
}

void VersionManageWindow::onCellDoubleClicked(const QModelIndex &index)
{
    int row = index.row();
    int col = index.column();

    // 获取所有版本和分组数据
    QList<GroupManageData> groupDataList = GroupDataManager::getInstance().getAllGroupData();
    QList<VersionManageData> &versionDataList = VersionDataManager::getInstance().getAllVersionData();

    if (row == 0 && col >= colOffset) { // 双击版本描述的单元格
        QString versionId = versionDataList[col - 2].versionId;
        bool ok;
        QString newDescription = QInputDialog::getText(this, "修改版本描述", "请输入新的版本描述：", QLineEdit::Normal, versionDataList[col - 2].versionDescription, &ok);

        if (ok && !newDescription.isEmpty()) {
            // 调用 updateVersionDescription 修改版本描述
            VersionDataManager::getInstance().updateVersionDescription(versionId, newDescription);
            // 刷新表格
            refresh();
        }
    } else if (col >= colOffset && row >= rowOffset) { // 双击分组是否启用的单元格
        int groupId = groupDataList[row - rowOffset].groupId;
        QString versionId = versionDataList[col - colOffset].versionId;
        bool isEnabled = versionDataList[col - colOffset].groupIds.contains(groupId);
        // 弹出提示框询问用户是否确认修改
        if(versionDataList[col - colOffset].isAverage) {
            QString message = QString("版本 %1 的 组 %2 由 %3 转为 %4")
                                  .arg(versionId)
                                  .arg(groupId)
                                  .arg(isEnabled ? "开启" : "关闭")
                                  .arg(isEnabled ? "关闭" : "开启");

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "修改功能状态", message, QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                // 调用 updateVersionGroupIds 修改分组启用状态
                VersionDataManager::getInstance().updateVersionGroupIds(versionId, groupId);
                if(isEnabled) {
                    versionDataList[col - colOffset].groupWeights.erase(groupId);
                    qDebug() << "erase:" << groupId;
                }
                float weight = 1.0 / versionDataList[col - colOffset].groupIds.size();
                for(const auto& groupId: versionDataList[col - colOffset].groupIds) {
                    versionDataList[col - colOffset].groupWeights[groupId] = weight;
                }
                TopLevelManager::getInstance().saveData();
                // 刷新表格
                refresh();
            }
        } else {
            QDialog *dialog = new QDialog(this);
            dialog->setWindowTitle("设置分组启用状态与权重");

            // 创建控件
            QCheckBox *checkBox = new QCheckBox(dialog);
            checkBox->setChecked(versionDataList[col - colOffset].groupIds.contains(groupId));  // 默认状态

            QLineEdit *weightEdit = new QLineEdit(dialog);

            weightEdit->setText(QString::number(versionDataList[col - colOffset].groupWeights[groupId] * 100, 'f', 2)); // 显示当前权重（以百分比显示）

            QPushButton *okButton = new QPushButton("确定", dialog);

            // 设置布局
            QVBoxLayout *layout = new QVBoxLayout(dialog);

            // 第一行：是否启用分组
            QHBoxLayout *firstRowLayout = new QHBoxLayout();
            firstRowLayout->addWidget(new QLabel("是否启用分组", dialog));
            firstRowLayout->addWidget(checkBox);
            layout->addLayout(firstRowLayout);

            // 第二行：权重和百分号
            QHBoxLayout *secondRowLayout = new QHBoxLayout();
            QLabel* weightLabel = new QLabel("权重", dialog);
            QLabel* percentLabel = new QLabel("%", dialog);
            secondRowLayout->addWidget(weightLabel);
            secondRowLayout->addWidget(weightEdit);
            secondRowLayout->addWidget(percentLabel);

            layout->addLayout(secondRowLayout);

            if(!versionDataList[col - colOffset].groupIds.contains(groupId)) {
                weightLabel->setVisible(false);
                weightEdit->setVisible(false);
                percentLabel->setVisible(false);
            }

            // 第三行：确定按钮
            layout->addWidget(okButton);

            dialog->setLayout(layout);

            connect(checkBox, &QCheckBox::toggled, [weightLabel, weightEdit, percentLabel](bool checked) {
                if (checked) {
                    weightLabel->setVisible(true);
                    weightEdit->setVisible(true);
                    percentLabel->setVisible(true);
                } else {
                    weightLabel->setVisible(false);
                    weightEdit->setVisible(false);
                    percentLabel->setVisible(false);
                }
            });

            connect(okButton, &QPushButton::clicked, [this, col, &versionDataList, dialog, checkBox, weightEdit, groupId, versionId]() {
                bool newIsEnabled = checkBox->isChecked();
                float newWeight = weightEdit->text().toFloat();

                // 更新分组启用状态
                if (newIsEnabled) {
                    if (!versionDataList[col - colOffset].groupIds.contains(groupId)) {
                        versionDataList[col - colOffset].groupIds.append(groupId);  // 启用该分组
                    }

                    // 更新权重（在启用分组时，添加或更新权重）
                    versionDataList[col - colOffset].groupWeights[groupId] = newWeight / 100.0;
                } else {
                    versionDataList[col - colOffset].groupIds.removeAll(groupId);  // 关闭该分组

                    // 删除对应的权重
                    versionDataList[col - colOffset].groupWeights.erase(groupId);
                }

                // 保存数据并刷新表格
                TopLevelManager::getInstance().saveData();
                refresh();

                dialog->accept();  // 关闭弹窗
            });

            dialog->exec();  // 弹出模态对话框
        }
    }
}
