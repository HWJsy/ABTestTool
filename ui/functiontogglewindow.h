#ifndef FUNCTIONTOGGLEWINDOW_H
#define FUNCTIONTOGGLEWINDOW_H

#include <QDialog>
#include <QTableView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QAction>
#include <QContextMenuEvent>
#include "newfunctionwindow.h"
namespace Ui {
class FunctionToggleWindow;
}

class FunctionToggleWindow : public QDialog
{
    Q_OBJECT

public:
    explicit FunctionToggleWindow(QWidget *parent = nullptr);
    ~FunctionToggleWindow();
    void refresh();

private slots:
    void on_pushButtonCreateFunction_clicked();
    void on_pushButtonMutexSwitch_clicked();
    void onCellDoubleClicked(const QModelIndex &index);

private:
    Ui::FunctionToggleWindow *ui;
    QTableView *tableViewFunctionList;
    QPushButton *pushButtonCreateFunction;
    void contextMenuEvent(QContextMenuEvent *event);
    void onDeleteRow();
    bool eventFilter(QObject *watched, QEvent *event);
    void removeFunctionData(const QString &functionName);
    bool checkMutex(QString functionName, MutexSwitchData currentMutexSwitch);
};

#endif // FUNCTIONTOGGLEWINDOW_H
