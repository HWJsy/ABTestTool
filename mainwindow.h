#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShowEvent>
#include <QFileOpenEvent>
#include "functiontogglewindow.h"
#include "groupmanagewindow.h"
#include "versionmanagewindow.h"
#include "groupupdatemappingwindow.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void refresh();
    void loadSpecificFile(const QString &filePath);

protected:
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;

private slots:
    void on_actionProjSetting_triggered();
    void on_actionNewFile_triggered();
    void on_actionOpenFile_triggered();
    void on_actionExport_triggered();

private:
    Ui::MainWindow *ui;
    FunctionToggleWindow *functionToggleWindow;  // 打开功能开关页面
    GroupManageWindow *groupManageWindow;      // 打开分组管理页面
    VersionManageWindow *versionManageWindow;  // 打开版本管理页面
    GroupUpdateMappingWindow *groupUpdateWindow; // 打开分组更新映射表页面
    bool fileLoadedFromCommandLine = false;  // 标记是否通过命令行参数加载了文件
};
#endif // MAINWINDOW_H
