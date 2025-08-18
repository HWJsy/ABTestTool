#include "projsetting.h"
#include "ui_projsetting.h"
#include "toplevelmanager.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

ProjSetting::ProjSetting(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjSetting)
{
    ui->setupUi(this);
    ui->lineEditCppExportPath->setReadOnly(true);  // 设置lineEdit不可编辑

    // 从 QSettings 读取保存的路径
    QSettings settings("App");
    QString savedPath = settings.value("cppExportPath").toString();

      // 将 savedPath 转换为绝对路径
    if (!savedPath.isEmpty()) {
        QDir dir(savedPath);
        QString absolutePath = dir.absolutePath();

        // 设置到 lineEdit 中
        ui->lineEditCppExportPath->setText(absolutePath);
    }
}

ProjSetting::~ProjSetting()
{
    delete ui;
}

void ProjSetting::on_pushButtonCppExportPath_clicked()
{
    QString currentDir = ui->lineEditCppExportPath->text().isEmpty()
    ? QDir::currentPath()  // 如果没有保存路径，则使用当前工作目录
    : ui->lineEditCppExportPath->text();

    // 弹出文件夹选择器，选择导出路径
    QString dir = QFileDialog::getExistingDirectory(this, "选择导出路径", currentDir);

    // 如果用户选择了路径
    if (!dir.isEmpty()) {
        // 设置导出路径到lineEdit
        ui->lineEditCppExportPath->setText(dir);

        // 使用 QSettings 保存路径
        QSettings settings("App");
        settings.setValue("cppExportPath", dir);  // 保存选择的目录路径

        TopLevelManager::getInstance().saveData();
    }
}
