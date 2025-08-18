#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "functiontogglewindow.h"
#include "groupmanagewindow.h"
#include "versionmanagewindow.h"
#include "groupupdatemappingwindow.h"
#include "projsetting.h"
#include "exportcpp.h"
#include "loadConfig.h"
#include <QVBoxLayout>
#include "toplevelmanager.h"
#include <QFileDialog>
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 不在构造函数中自动加载数据，等待明确的文件加载指令

    QTabWidget *tabWidget = new QTabWidget(this); // 创建 QTabWidget

    // 创建每个模块的 QWidget 内容
    functionToggleWindow = new FunctionToggleWindow();
    groupManageWindow = new GroupManageWindow();
    versionManageWindow = new VersionManageWindow();
    groupUpdateWindow = new GroupUpdateMappingWindow();

    // 将这些 QWidget 添加到 Tab 页中
    tabWidget->addTab(functionToggleWindow, "功能开关");
    tabWidget->addTab(groupManageWindow, "分组管理");
    tabWidget->addTab(versionManageWindow, "版本管理");
    tabWidget->addTab(groupUpdateWindow, "分组更新");

    // 设置 QTabWidget 为主窗口的中心控件
    setCentralWidget(tabWidget);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    // 只在第一次显示时执行，且没有通过命令行参数加载文件时
    static bool firstShow = true;
    if (firstShow && !fileLoadedFromCommandLine)
    {
        firstShow = false;

        // 加载上次保存的文件
        QSettings settings("App");
        QString currentPath = settings.value("configFilePath").toString();

        if (!currentPath.isEmpty() && QFile::exists(currentPath))
        {
            TopLevelManager::getInstance().loadData();
            refresh();

            QFileInfo fileInfo(currentPath);
            setWindowTitle(QString("AB Test Tool - %1").arg(fileInfo.fileName()));
        }
    }
}

void MainWindow::on_actionProjSetting_triggered()
{
    // 创建并显示 PathSettingWindow 窗口
    ProjSetting *pathSettingWindow = new ProjSetting(this);
    pathSettingWindow->exec(); // 以模态方式显示对话框
}

MainWindow::~MainWindow()
{
    TopLevelManager::getInstance().saveData();
    delete ui;
}

void MainWindow::refresh()
{
    TopLevelManager::getInstance().loadData();
    functionToggleWindow->refresh();
    groupManageWindow->refresh();
    versionManageWindow->refresh();
    groupUpdateWindow->refresh();
}

void MainWindow::on_actionExport_triggered()
{
    LoadConfig::getInstance().loadConfig();

    auto warningMessage = ExportCpp::getInstance().checkMutex();

    // 如果有冲突，弹出提示框并显示所有的冲突信息
    if (!warningMessage.isEmpty())
    {
        QMessageBox::warning(this, "互斥开关冲突", warningMessage);
        return;
    }

    if (versionManageWindow->isTotalWeightsOver1())
    {
        QMessageBox::warning(this, "总权重不等于100%", "最新版本总权重不等于100%，无法导出");
        return;
    }

    QSettings settings("App");
    QString exportPath = settings.value("cppExportPath").toString();

    // 检查导出路径是否为空
    if (exportPath.isEmpty())
    {
        QMessageBox::warning(this, "导出路径未设置", "请先设置导出路径！");
        return;
    }

    if (ExportCpp::getInstance().exportHeader() && ExportCpp::getInstance().exportCpp())
    {
        QMessageBox::information(this, "导出成功", "已成功导出！");
    }
    else
    {
        QMessageBox::warning(this, "导出失败", "导出过程中出现错误，请检查！");
    }
}

void MainWindow::on_actionNewFile_triggered()
{
    // 获取当前文件路径（如果有的话）
    QSettings settings("App");
    QString currentPath = settings.value("configFilePath").toString();

    // 如果没有当前路径，则使用空字符串（表示默认目录）
    QString dir = currentPath.isEmpty() ? "" : QFileInfo(currentPath).absolutePath();

    // 打开文件保存对话框，默认文件类型为 redabproj 文件
    QString filePath = QFileDialog::getSaveFileName(this, "创建新文件", currentPath, "ab Project Files (*.abproj)");

    // 如果用户没有选择文件，返回
    if (filePath.isEmpty())
    {
        return;
    }

    // 自动补全 .abproj 后缀
    if (!filePath.endsWith(".abproj", Qt::CaseInsensitive))
    {
        filePath.append(".abproj");
    }

    // 创建一个空的 JSON 文件
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
    {
        // 创建一个空的 JSON 对象作为文件内容
        QJsonObject jsonObject;
        QJsonDocument doc(jsonObject);

        // 写入 JSON 数据到文件
        file.write(doc.toJson());
        file.close();

        // 将文件路径保存到 QSettings 中
        QSettings settings("App");
        settings.setValue("configFilePath", filePath);

        // 更新窗口标题
        QFileInfo fileInfo(filePath);
        setWindowTitle(QString("AB Test Tool - %1").arg(fileInfo.fileName()));

        LoadConfig::getInstance().loadConfig();
        refresh();
        // 提示用户文件创建成功
        QMessageBox::information(this, "成功", "新文件已创建！");
    }
    else
    {
        // 显示错误消息
        QMessageBox::critical(this, "错误", "无法创建文件！");
    }
}

void MainWindow::on_actionOpenFile_triggered()
{
    // 获取上次保存的文件路径（如果有的话）
    QSettings settings("App");
    QString lastPath = settings.value("configFilePath").toString();

    // 如果没有上次路径，则使用空字符串（表示默认目录）
    QString dir = lastPath.isEmpty() ? "" : QFileInfo(lastPath).absolutePath();

    // 打开文件选择对话框，限制文件类型为 redabproj 文件，并设置默认路径
    QString filePath = QFileDialog::getOpenFileName(this, "打开文件", dir, "ab Project Files (*.abproj)");

    // 如果用户没有选择文件，返回
    if (filePath.isEmpty())
    {
        return;
    }

    // 将选择的文件路径保存到 QSettings 中
    settings.setValue("configFilePath", filePath);
    LoadConfig::getInstance().loadConfig();

    // 更新窗口标题
    QFileInfo fileInfo(filePath);
    setWindowTitle(QString("AB Test Tool - %1").arg(fileInfo.fileName()));

    refresh();
}

void MainWindow::loadSpecificFile(const QString &filePath)
{
    // 验证文件是否存在
    if (!QFile::exists(filePath))
    {
        QMessageBox::warning(this, "文件不存在", QString("无法找到文件: %1").arg(filePath));
        return;
    }

    // 验证文件扩展名
    if (!filePath.endsWith(".abproj", Qt::CaseInsensitive))
    {
        QMessageBox::warning(this, "文件类型错误", "只能打开 .abproj 文件");
        return;
    }

    // 标记已通过命令行参数加载文件
    fileLoadedFromCommandLine = true;

    // 将文件路径保存到 QSettings 中
    QSettings settings("App");
    settings.setValue("configFilePath", filePath);

    // 加载数据并刷新界面
    TopLevelManager::getInstance().loadData();
    refresh();

    // 更新窗口标题显示当前打开的文件
    QFileInfo fileInfo(filePath);
    setWindowTitle(QString("AB Test Tool - %1").arg(fileInfo.fileName()));
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        QString fileName = openEvent->file();

        if (!fileName.isEmpty())
        {
            loadSpecificFile(fileName);
            return true;
        }
    }
    return QMainWindow::event(event);
}
