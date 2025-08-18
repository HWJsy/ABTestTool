#include "newversionwindow.h"
#include "ui_newversionwindow.h"
#include "versiondatamanager.h"
#include <QMessageBox>

newVersionWindow::newVersionWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::newVersionWindow)
{
    ui->setupUi(this);
}

newVersionWindow::~newVersionWindow()
{
    delete ui;
}

void newVersionWindow::on_buttonBox_accepted()
{
    QString versionId = ui->lineEditVersionId->text();  // 获取版本号
    QString versionDescription = ui->textEditVersionDescription->toPlainText();  // 获取版本描述

    // 检查版本号和版本描述是否为空
    if (versionId.isEmpty() || versionDescription.isEmpty()) {
        QMessageBox::warning(this, "输入不完整", "版本号和版本描述不能为空！");
        return;
    }

    // 如果两个字段都已填入，调用 VersionDataManager 的 addVersionData 函数
    VersionDataManager::getInstance().addVersionData(versionId, versionDescription);

    accept();
}

void newVersionWindow::on_buttonBox_rejected()
{
    reject();
}
