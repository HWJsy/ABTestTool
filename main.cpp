#include "mainwindow.h"

#include <QApplication>
#include <QFileOpenEvent>

class MyApplication : public QApplication
{
public:
    MyApplication(int &argc, char **argv) : QApplication(argc, argv), mainWindow(nullptr) {}
    
    void setMainWindow(MainWindow *window) { mainWindow = window; }
    
protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            QString fileName = openEvent->file();
            
            if (mainWindow && !fileName.isEmpty()) {
                mainWindow->loadSpecificFile(fileName);
                return true;
            }
        }
        return QApplication::event(event);
    }
    
private:
    MainWindow *mainWindow;
};

int main(int argc, char *argv[])
{
    MyApplication a(argc, argv);
    
    // 检查是否有文件作为命令行参数传入
    QString fileToOpen;
    if (argc > 1) {
        fileToOpen = QString::fromLocal8Bit(argv[1]);
    }
    
    MainWindow w;
    a.setMainWindow(&w);
    
    // 如果有文件参数，则传递给主窗口
    if (!fileToOpen.isEmpty()) {
        w.loadSpecificFile(fileToOpen);
    }
    
    w.show();
    return a.exec();
}
