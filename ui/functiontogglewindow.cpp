#include "functiontogglewindow.h"
#include "ui_functiontogglewindow.h"
#include "groupmanagewindow.h"
#include "groupdatamanager.h"
#include "newmutexswitchwindow.h"
#include <QInputDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QRegExp>
#include <QMessageBox>
#include <QStackedWidget>
#include "toplevelmanager.h"
#include <QSettings>
FunctionToggleWindow::FunctionToggleWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FunctionToggleWindow)
{
    ui->setupUi(this);
    setWindowTitle("功能开关模块");

    tableViewFunctionList = ui->tableViewFunctionList;
    QStandardItemModel *model = new QStandardItemModel();
    model->setColumnCount(5);  // 设置列数为4
    model->setHorizontalHeaderLabels({"功能名称", "函数名称", "返回类型", "默认值", "互斥开关"});
    tableViewFunctionList->setModel(model);
    // 禁用编辑
    ui->tableViewFunctionList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHeaderView *header = tableViewFunctionList->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    connect(tableViewFunctionList, &QTableView::doubleClicked, this, &FunctionToggleWindow::onCellDoubleClicked);
    ui->tableViewFunctionList->installEventFilter(this);
    refresh();
}

FunctionToggleWindow::~FunctionToggleWindow()
{
    delete ui;
}

void FunctionToggleWindow::on_pushButtonCreateFunction_clicked()
{

    QSettings settings("App");
    QString savedPath = settings.value("configFilePath").toString();

    // 判断路径是否为空
    if (savedPath.isEmpty()) {
        // 路径为空，提示用户设置路径
        QMessageBox::warning(this, "路径未设置", "请先设置配置文件路径！");
        return;  // 不继续执行，等待用户设置路径
    }
    // 打开新建功能窗口
    newFunctionWindow newFunctionWindow;

    // 判断用户是否点击了“确定”按钮（QDialog::Accepted）
    newFunctionWindow.exec();
    refresh();
}

void FunctionToggleWindow::on_pushButtonMutexSwitch_clicked()
{
    NewMutexSwitchWindow newMutexSwitchWindow;
    newMutexSwitchWindow.exec();
    refresh();
}

bool FunctionToggleWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        refresh();
    }
    return QDialog::eventFilter(watched, event); // 返回给父类处理
}

void FunctionToggleWindow::refresh()
{
    // 获取 FunctionDataManager 的单例
    FunctionDataManager &dataManager = FunctionDataManager::getInstance();

    TopLevelManager::getInstance().loadData();

    // 获取所有功能数据
    QList<FunctionData> allFunctions = dataManager.getAllFunctionData();

    // 获取当前表格模型
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(tableViewFunctionList->model());
    model->removeRows(0, model->rowCount());  // 清空当前表格的数据

    // 获取所有互斥开关的数据
    QList<MutexSwitchData> mutexSwitches = FunctionDataManager::getInstance().getMutexSwitches();

    // 填充新数据
    for (FunctionData &data : allFunctions) {
        // 创建每一行
        QList<QStandardItem *> rowItems;

        rowItems.append(new QStandardItem(data.functionDescription));
        rowItems.append(new QStandardItem(data.functionName));

        // 功能状态：转换为 "默认开启" 或 "默认关闭"
        rowItems.append(new QStandardItem(data.returnType));
        QString defaultKey;
        if(data.returnType == "bool") {
            defaultKey = data.currentKeyValueId == 0? "false": "true";
        } else {
            defaultKey = data.keyValuePairs[data.currentKeyValueId].first;
        }
        rowItems.append(new QStandardItem(defaultKey));

        // 设置第五列：互斥开关
        QString mutexSwitchDescription = ""; // 默认没有互斥开关

        // 仅当返回类型为 bool 时，才设置互斥开关
        if (data.returnType == "bool") {
            // 遍历所有互斥开关，找到包含该函数名的开关，并设置其描述
            for (const MutexSwitchData &mutexSwitch : mutexSwitches) {
                if (mutexSwitch.functions.find(data.functionName) != mutexSwitch.functions.end()) {
                    // 如果该函数名存在于互斥开关的 functions 中，设置该互斥开关的描述
                    mutexSwitchDescription = mutexSwitch.description;
                    break;
                }
            }
        }

        rowItems.append(new QStandardItem(mutexSwitchDescription)); // 将互斥开关描述添加到第五列

        // 将行数据添加到模型中
        model->appendRow(rowItems);
    }
}

void FunctionToggleWindow::contextMenuEvent(QContextMenuEvent *event)
{
    // 获取点击的单元格的索引
    QModelIndex index = ui->tableViewFunctionList->indexAt(event->pos());

    // 如果点击的是有效的单元格（不是表头）
    if (index.isValid()) {
        // 创建右键菜单
        QMenu contextMenu(this);

        // 创建删除操作
        QAction *deleteAction = new QAction("删除该行", this);
        connect(deleteAction, &QAction::triggered, this, &FunctionToggleWindow::onDeleteRow);

        // 将删除操作添加到右键菜单
        contextMenu.addAction(deleteAction);

        // 显示右键菜单
        contextMenu.exec(event->globalPos());
    }
}

void FunctionToggleWindow::onDeleteRow()
{
    // 获取当前选中的行
    int currentRow = ui->tableViewFunctionList->currentIndex().row();

    // 获取该行的功能名称
    QString functionName = ui->tableViewFunctionList->model()->data(ui->tableViewFunctionList->model()->index(currentRow, 0)).toString();

    // 弹出确认删除框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "删除功能", QString("是否删除功能：%1?").arg(functionName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 调用删除函数，删除该项
        removeFunctionData(functionName);

        // 从表格模型中删除这一行
        ui->tableViewFunctionList->model()->removeRow(currentRow);
    }
}

void FunctionToggleWindow::removeFunctionData(const QString &functionName)
{
    // 调用 FunctionDataManager 中的 removeFunctionData 来删除数据
    FunctionDataManager::getInstance().removeFunctionData(functionName);
}

void FunctionToggleWindow::onCellDoubleClicked(const QModelIndex &index)
{
    int row = index.row();
    int col = index.column();

    // 获取模型数据
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(tableViewFunctionList->model());

    // 获取当前行的数据
    QString originalDescription = model->data(model->index(row, 0)).toString();  // 功能名称
    QString originalFunctionName = model->data(model->index(row, 1)).toString();  // 函数名称
    QString originalReturnType = model->data(model->index(row, 2)).toString();   // 返回类型
    QString originalReturnValue = model->data(model->index(row, 3)).toString();  // 返回值

    // 获取当前行所属的 FunctionData
    FunctionData rowData;
    for (const auto& it : FunctionDataManager::getInstance().getFunctionDataList()) {
        if (it.functionName == originalFunctionName) {
            rowData = it;
            break;
        }
    }

    // 弹出一个对话框，供用户编辑整行数据
    QDialog editDialog(this);
    QFormLayout formLayout(&editDialog);

    QLineEdit *lineEditDescription = new QLineEdit(originalDescription);
    QLabel *FunctionName = new QLabel(originalFunctionName);
    QLabel *returnType = new QLabel(originalReturnType);

    formLayout.addRow("功能名称:", lineEditDescription);
    formLayout.addRow("函数名称:", FunctionName);
    formLayout.addRow("返回类型:", returnType);

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    // 默认值

    QComboBox *comboBoxValue = new QComboBox();
    horizontalLayout->addWidget(comboBoxValue);

    // 创建新按钮，并设置显示条件
    QPushButton *newButton = new QPushButton("新增键值对");
    newButton->setVisible(originalReturnType != "bool");

    // 添加一个弹性间距，确保 newButton 始终位于水平布局的最右侧
    QSpacerItem *spacer = new QSpacerItem(200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    horizontalLayout->addWidget(newButton);

    // 将水平布局添加到表单中
    formLayout.addRow("默认值:", horizontalLayout);

    // 创建 tableView 来显示 keyValuePairs
    QTableView *keyValueTableView = new QTableView();
    QStandardItemModel *keyValueModel = new QStandardItemModel(rowData.keyValuePairs.size(), 2, this);
    // 填充 tableView 的模型数据
    int rowIndex = 0;
    for(int id = 0; id < rowData.keyValuePairs.size(); id++) {
        comboBoxValue->addItem(rowData.keyValuePairs[id].first);
        keyValueModel->setItem(rowIndex, 0, new QStandardItem(rowData.keyValuePairs[id].first));
        keyValueModel->setItem(rowIndex, 1, new QStandardItem(rowData.keyValuePairs[id].second));
        rowIndex++;
    }
    comboBoxValue->setCurrentText(rowData.keyValuePairs[rowData.currentKeyValueId].first);
    keyValueTableView->setModel(keyValueModel);
    keyValueModel->setHeaderData(0, Qt::Horizontal, "Key");
    keyValueModel->setHeaderData(1, Qt::Horizontal, "Value");
    keyValueTableView->horizontalHeader()->setStretchLastSection(true);  // 确保最后一列填满剩余空间
    keyValueTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);  // 第一列自适应
    keyValueTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);  // 第二列自适应
    keyValueTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    if(originalReturnType != "bool") {
        formLayout.addRow(keyValueTableView);
    }

    // 创建互斥开关标签和下拉框
    QLabel *mutexSwitchLabel = new QLabel("互斥开关:");
    QComboBox *mutexSwitchComboBox = new QComboBox();
    MutexSwitchData* originMutexSwitch = nullptr;
    QList<MutexSwitchData>& mutexSwitches = FunctionDataManager::getInstance().getMutexSwitches();
    // 当返回类型为 bool 时，添加“互斥开关”下拉框
    if (originalReturnType == "bool") {
        // 添加第一项为空选项
        mutexSwitchComboBox->addItem("");  // 第一项是“空”
        // 将所有互斥开关填充到下拉框中
        for (MutexSwitchData &mutexSwitch : mutexSwitches) {
            mutexSwitchComboBox->addItem(mutexSwitch.description);  // 填充互斥开关的描述

            // 检查函数名是否在当前互斥开关的 functions 中
            if (mutexSwitch.functions.find(originalFunctionName) != mutexSwitch.functions.end()) {
                // 如果包含当前行的函数名，设置当前值为该互斥开关的描述
                mutexSwitchComboBox->setCurrentText(mutexSwitch.description);
                originMutexSwitch = &mutexSwitch;
            }
        }

        // 将互斥开关标签和下拉框添加到表单中
        formLayout.addRow(mutexSwitchLabel, mutexSwitchComboBox);
    }

    connect(mutexSwitchComboBox, &QComboBox::currentTextChanged, [this, &mutexSwitchComboBox, &originMutexSwitch, &mutexSwitches, originalFunctionName](const QString &currentDescription) {
        // 获取新选择的互斥开关
        MutexSwitchData* currentMutexSwitch = nullptr;

        for (MutexSwitchData &mutexSwitch : mutexSwitches) {
            if (mutexSwitch.description == currentDescription) {
                currentMutexSwitch = &mutexSwitch;
                break;
            }
        }

        if (originMutexSwitch == currentMutexSwitch) {
            return;  // 如果选择没有变化，直接返回
        }

        // 将函数名添加到新选择的互斥开关中
        if (currentMutexSwitch) {
            if(!checkMutex(originalFunctionName, *currentMutexSwitch)) {
                QString originText = originMutexSwitch ? originMutexSwitch->description : "";
                mutexSwitchComboBox->setCurrentText(originText);
                return;
            }

            currentMutexSwitch->functions.insert(originalFunctionName);
        }

        // 从原始互斥开关中删除函数名
        if (originMutexSwitch) {
            originMutexSwitch->functions.erase(originalFunctionName);
        }

        originMutexSwitch = currentMutexSwitch;

        TopLevelManager::getInstance().saveData();
    });

    connect(comboBoxValue, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&comboBoxValue, &rowData] {
        for(auto& it: rowData.keyValuePairs) {
            if(it.second.first == comboBoxValue->currentText()) {
                rowData.currentKeyValueId = it.first;
                break;
            }
        }
    });


    // 创建新键值对
    connect(newButton, &QPushButton::clicked, this, [this, originalReturnType, &rowData, &keyValueTableView, &FunctionName, &comboBoxValue] {
        // 弹出新对话框
        qDebug() << rowData.currentKeyValueId;
        QDialog inputDialog(this);
        inputDialog.setWindowTitle("输入键值对");

        QFormLayout dialogLayout(&inputDialog);

        QLineEdit *lineEditKey = new QLineEdit();
        QLineEdit *lineEditValue = new QLineEdit();

        dialogLayout.addRow("键:", lineEditKey);
        dialogLayout.addRow("值:", lineEditValue);

        QPushButton *addButton = new QPushButton("添加");
        dialogLayout.addRow(addButton);

        connect(addButton, &QPushButton::clicked, this, [this, originalReturnType,&inputDialog, &lineEditKey, &lineEditValue, &rowData, &keyValueTableView, &FunctionName, &comboBoxValue] {
            QString key = lineEditKey->text();
            QString value = lineEditValue->text();

            bool isValid = true;

            // 如果键和值不为空
            if (!key.isEmpty() && !value.isEmpty()) {
                // 根据返回类型检查值是否合法
                QString returnType = originalReturnType;

                if (returnType == "int") {
                    // 检查是否为整数
                    bool ok;
                    value.toInt(&ok);
                    if (!ok) {
                        isValid = false;
                        QMessageBox::warning(this, "警告", "值必须是一个有效的整数！");
                    }
                } else if (returnType == "float") {
                    // 检查是否为浮点数
                    bool ok;
                    value.toFloat(&ok);
                    if (!ok) {
                        isValid = false;
                        QMessageBox::warning(this, "警告", "值必须是一个有效的浮点数！");
                    }
                }

                if (isValid) {
                    bool isExist = 0;
                    for(auto& it: rowData.keyValuePairs) {
                        if(it.second.first == key) {
                            isExist = 1;
                            break;
                        }
                    }
                    if (!isExist) {
                        // 如果键不存在，添加到 keyValuePairs
                        rowData.keyValuePairs[rowData.keyValuePairs.size()] = std::pair(key, value);
                        comboBoxValue->addItem(key);
                        // 更新 tableView
                        QStandardItemModel *keyValueModel = qobject_cast<QStandardItemModel*>(keyValueTableView->model());
                        int rowIndex = 0;
                        for(int id = 0; id < rowData.keyValuePairs.size(); id++) {
                            keyValueModel->setItem(rowIndex, 0, new QStandardItem(rowData.keyValuePairs[id].first));
                            keyValueModel->setItem(rowIndex, 1, new QStandardItem(rowData.keyValuePairs[id].second));
                            rowIndex++;
                        }

                        inputDialog.accept();
                    } else {
                        // 如果键已经存在，提示用户
                        QMessageBox::warning(this, "警告", "该键已存在！");
                    }
                }
            } else {
                // 如果键或值为空，提示用户
                QMessageBox::warning(this, "警告", "键和值不能为空！");
            }
        });

        inputDialog.exec();
    });


    connect(keyValueTableView, &QTableView::doubleClicked, this, [this, &comboBoxValue, originalReturnType, &keyValueTableView, &rowData, FunctionName](const QModelIndex &index) {
        int id = index.row();

        // 获取模型数据
        QStandardItemModel *model = qobject_cast<QStandardItemModel*>(keyValueTableView->model());
        QString returnType = originalReturnType;

        // 如果返回类型是 bool，直接返回
        if (returnType == "bool") {
            return;
        }

        // 获取当前行的数据
        QString originalKey = rowData.keyValuePairs[id].first;  // 键
        QString originalValue = rowData.keyValuePairs[id].second;  // 值

        // 弹出编辑窗口
        QDialog editDialog(this);
        editDialog.setWindowTitle("编辑键值对");
        QFormLayout formLayout(&editDialog);

        // 创建两个 QLineEdit 控件，默认填入点击行的 key 和 value
        QLineEdit *lineEditKey = new QLineEdit(originalKey);
        QLineEdit *lineEditValue = new QLineEdit(originalValue);

        formLayout.addRow("键:", lineEditKey);
        formLayout.addRow("值:", lineEditValue);

        QPushButton *okButton = new QPushButton("确定");
        formLayout.addWidget(okButton);

        // 点击“确定”按钮时执行以下操作
        connect(okButton, &QPushButton::clicked, this, [this, &comboBoxValue, originalReturnType, &editDialog, &lineEditKey, &lineEditValue, id, model, &rowData, originalKey, originalValue, FunctionName] {
            QString newKey = lineEditKey->text();
            QString newValue = lineEditValue->text();

            // 检查 key 和 value 是否为空
            if (newKey.isEmpty() || newValue.isEmpty()) {
                QMessageBox::warning(this, "输入无效", "键和值不能为空！");
                return;  // 如果为空，直接返回
            }

            // 根据返回类型检查值是否合法
            QString returnType = originalReturnType;

            bool isValid = true;

            // 如果返回类型是 int，检查 value 是否为合法的整数
            if (returnType == "int") {
                bool ok;
                newValue.toInt(&ok);
                if (!ok) {
                    isValid = false;
                    QMessageBox::warning(this, "警告", "值必须是一个有效的整数！");
                    return;
                }
            }
            // 如果返回类型是 float，检查 value 是否为合法的浮点数
            else if (returnType == "float") {
                bool ok;
                newValue.toFloat(&ok);
                if (!ok) {
                    isValid = false;
                    QMessageBox::warning(this, "警告", "值必须是一个有效的浮点数！");
                    return;
                }
            }
            bool isExist = 0;
            for(auto& it: rowData.keyValuePairs) {
                if(it.second.first == originalKey) {
                    continue;
                }
                if(it.second.first == newKey) {
                    isExist = 1;
                    break;
                }
            }
            if(isExist) {
                QMessageBox::warning(this, "警告", "键不能重复！");
                return;
            }

            // 如果值合法，更新数据
            if (isValid) {
                rowData.keyValuePairs[id] = std::pair(newKey, newValue);  // 添加新的键值对

                // 更新表格中的显示
                model->setData(model->index(id, 0), newKey);
                model->setData(model->index(id, 1), newValue);

                // 更新下拉栏
                int index = comboBoxValue->findText(originalKey);
                comboBoxValue->setItemText(index, newKey);
                comboBoxValue->setCurrentIndex(index);

                // 关闭对话框
                editDialog.accept();
            }
        });

        // 显示对话框并等待用户输入
        editDialog.exec();
    });

    QPushButton *okButton = new QPushButton("确定", &editDialog);
    formLayout.addWidget(okButton);

    connect(okButton, &QPushButton::clicked, this, [this, &mutexSwitchComboBox, &editDialog, &rowData,lineEditDescription, FunctionName, returnType, row, col, model, originalDescription, originalFunctionName, originalReturnType, originalReturnValue]() {
        QString newDescription = lineEditDescription->text();
        QString newFunctionName = FunctionName->text();
        QString newReturnValue = rowData.keyValuePairs[rowData.currentKeyValueId].second;


        // 检查 newDescription 和 newFunctionName 是否为空
        if (newDescription.isEmpty() || newFunctionName.isEmpty()) {
            QMessageBox::warning(this, "输入无效", "功能名称和函数名称不能为空！");
            return;  // 如果为空，直接返回
        }

        // 验证函数名称是否符合 C++ 标准
        QRegExp functionNameRegex("^[a-zA-Z_][a-zA-Z0-9_]*$");
        if (!functionNameRegex.exactMatch(newFunctionName)) {
            QMessageBox::warning(this, "无效的函数名称", "函数名称不符合 C++ 标准。请确保以字母或下划线开头，并且仅包含字母、数字和下划线。");
            return;  // 如果函数名称不合法，直接返回
        }

        if(newReturnValue.isEmpty()) {
            QMessageBox::warning(this, "无默认值", "请创建并选择一个默认值");
            return;
        }
        rowData.functionDescription = newDescription;
        rowData.functionName = newFunctionName;
        // 更新数据
        FunctionDataManager::getInstance().updateFunctionData(originalFunctionName, rowData);

        // 更新表格中的显示
        model->setData(model->index(row, 0), newDescription);
        model->setData(model->index(row, 1), newFunctionName);
        model->setData(model->index(row, 2), originalReturnValue);
        model->setData(model->index(row, 3), newReturnValue);


        // 刷新表格
        refresh();

        // 关闭对话框
        editDialog.accept();
    });

    // 显示对话框并等待用户输入
    editDialog.exec();
}

bool FunctionToggleWindow::checkMutex(QString functionName, MutexSwitchData currentMutexSwitch)
{
    if (currentMutexSwitch.functions.empty()) {
        return true;  // 如果当前互斥开关的 functions 为空，直接返回
    }

    std::map<QString, QString> warningInfo;

    const auto& groupList = GroupDataManager::getInstance().getGroupList();
    for (const auto& group : groupList) {
        // 如果该分组中没有启用该功能，那么跳过
        if (group.functions.at(functionName) == 0) {
            continue;
        }

        // 遍历互斥开关的所有功能，检查当前分组是否有启用的功能使用了该互斥开关
        for (const auto& function : currentMutexSwitch.functions) {
            if (group.functions.at(function) == 1) {
                // 如果分组有启用的功能使用了该互斥开关，将该分组和功能名称记录到警告信息中
                warningInfo[group.groupDescription] = function;
            }
        }
    }

    // 如果 warningInfo 中有数据，弹出警告框显示相关信息
    if (!warningInfo.empty()) {
        QString warningMessage = "存在使用同一个互斥开关且已启用的功能:\n\n";

        // 遍历 warningInfo，构建警告信息
        for (const auto& info : warningInfo) {
            QString groupName = info.first;  // 分组名称
            QString conflictingFunction = info.second;  // 冲突的功能名称
            warningMessage += QString("分组: %1，功能: %2\n").arg(groupName, conflictingFunction);
        }

        // 创建并显示警告框
        QMessageBox::warning(nullptr, "互斥开关冲突", warningMessage);
        return false;
    }

    return true;  // 如果没有冲突，返回 true
}



