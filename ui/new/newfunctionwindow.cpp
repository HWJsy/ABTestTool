#include "newfunctionwindow.h"
#include "ui_newfunctionwindow.h"
#include <QMessageBox>
#include <QRegExpValidator>
#include <QRegExp>
#include "groupdatamanager.h"
newFunctionWindow::newFunctionWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::newFunctionWindow)
{
    ui->setupUi(this);
    ui->comboBoxReturnType->addItem("bool");
    ui->comboBoxReturnType->addItem("int");
    ui->comboBoxReturnType->addItem("string");
    ui->comboBoxReturnType->addItem("float");
    ui->comboBoxDefaultValue->addItem("false");
    ui->comboBoxDefaultValue->addItem("true");

    // 连接 comboBoxReturnType 的变化信号到槽函数
    connect(ui->comboBoxReturnType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &newFunctionWindow::onReturnTypeChanged);

    // 初始设置
    onReturnTypeChanged(ui->comboBoxReturnType->currentIndex());
}

newFunctionWindow::~newFunctionWindow()
{
    delete ui;
}

FunctionData newFunctionWindow::getFunctionData() {
    FunctionData data;
    data.functionName = ui->lineEditFunctionName->text();
    data.functionDescription = ui->lineEditFunctionDescription->text();
    data.returnType = ui->comboBoxReturnType->currentText();
    if(data.returnType == "bool") {
        QString str = ui->comboBoxDefaultValue->currentText() == "true"? "1": "0";
        data.keyValuePairs[0] = std::pair("false", "0");
        data.keyValuePairs[1] = std::pair("true", "1");
        data.currentKeyValueId = str.toInt();
    } else {
        data.currentKeyValueId = 0;
        data.keyValuePairs[0] = std::pair(ui->lineEditDefaultKey->text(), ui->lineEditDefaultValue->text());
    }

    return data;
}

bool isFunctionNameAndDescriptionUnique(const QString &functionName, const QString &functionDescription)
{
    QList<FunctionData> existingFunctions = FunctionDataManager::getInstance().getAllFunctionData();
    for (const FunctionData &data : existingFunctions) {
        if (data.functionName == functionName) {
            return false;  // 如果功能名称重复，返回 false
        }
        if (data.functionDescription == functionDescription) {
            return false;  // 如果函数名称重复，返回 false
        }
    }
    return true;  // 如果没有相同的功能名称和函数名称，返回 true
}

void newFunctionWindow::on_buttonBox_accepted()
{
    // 获取用户输入的内容
    QString functionName = ui->lineEditFunctionName->text();
    QString functionDescription = ui->lineEditFunctionDescription->text();

    // 检查前两个输入框是否为空
    if (functionName.isEmpty() || functionDescription.isEmpty()) {
        QMessageBox::warning(this, tr("输入不完整"), tr("功能名称和函数名称不能为空！"));
        return; // 如果有字段为空，不继续执行
    }

    // 使用正则表达式检查函数名是否合法
    QRegExp regex("^[A-Za-z_][A-Za-z0-9_]*$");
    if (!regex.exactMatch(functionName)) {
        QMessageBox::warning(this, tr("无效的函数名称"), tr("函数名称无效！函数名必须以字母或下划线开头，且只能包含字母、数字和下划线。"));
        return;
    }

    // 检查功能名称是否唯一
    if (!isFunctionNameAndDescriptionUnique(functionName, functionDescription)) {
        QMessageBox::warning(this, "功能名称或函数名称重复", "功能名称或函数名称已存在，请使用不同的名称！");
        return;
    }

    QString returnType = ui->comboBoxReturnType->currentText();
    QString Value;
    QString Key;
    if(returnType != "bool") {
        Key = ui->lineEditDefaultKey->text();
        Value = ui->lineEditDefaultValue->text();
        if(Key.isEmpty() || Value.isEmpty()) {
            QMessageBox::warning(this, tr("输入不完整"), tr("Key和Value不能为空！"));
            return;
        }

        if (returnType == "int") {
            // 如果是 int 类型，检查返回值是否为整数
            bool ok;
            int intValue = Value.toInt(&ok);
            if (!ok) {
                QMessageBox::warning(this, tr("无效的默认值"), tr("value不匹配返回类型！"));
                return;
            }
        } else if (returnType == "float") {
            // 如果是 float 类型，检查返回值是否为浮点数
            bool ok;
            float floatValue = Value.toFloat(&ok);
            if (!ok) {
                QMessageBox::warning(this, tr("无效的默认值"), tr("value不匹配返回类型！"));
                return;
            }
        } else if (returnType == "string") {
            // string 不需要类型检查
        }
    } else {
        Value = ui->comboBoxDefaultValue->currentText();
        qDebug() << ui->comboBoxDefaultValue->currentText();
        qDebug() << Value;
    }

    // 创建功能数据结构，并存储用户输入
    FunctionData data = getFunctionData();


    QList<GroupManageData>& groupList = GroupDataManager::getInstance().getGroupList();
    for (GroupManageData &group : groupList) {
        group.functions[functionName] = data.currentKeyValueId;
    }

    // 将新功能添加到数据管理器
    FunctionDataManager::getInstance().addFunctionData(data);

    // 关闭对话框
    accept();
}

void newFunctionWindow::on_buttonBox_rejected()
{
    reject(); // 确保关闭对话框
}

void newFunctionWindow::onReturnTypeChanged(int index)
{
    // 判断返回值类型是否为 bool
    if (ui->comboBoxReturnType->currentText() == "bool") {
        ui->comboBoxDefaultValue->setVisible(true);   // 显示 comboBoxDefaultValue
        ui->lineEditDefaultKey->setVisible(false);  // 隐藏 lineEditDefaultValue
        ui->lineEditDefaultValue->setVisible(false);  // 隐藏 lineEditDefaultValue
        ui->label_Key->setText("默认值");
        ui->label_Value->setVisible(false);
    } else {
        ui->comboBoxDefaultValue->setVisible(false);  // 隐藏 comboBoxDefaultValue
        ui->lineEditDefaultKey->setVisible(true);   // 显示 lineEditDefaultValue
        ui->lineEditDefaultValue->setVisible(true);  // 隐藏 lineEditDefaultValue
        ui->label_Key->setText("默认Key");
        ui->label_Value->setVisible(true);
    }
}
