#ifndef GROUPMANAGEWINDOW_H
#define GROUPMANAGEWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QTableView>
#include <QFocusEvent> // 包含焦点事件的头文件
#include "newgroupwindow.h"

namespace Ui {
class GroupManageWindow;
}

class GroupManageWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GroupManageWindow(QWidget *parent = nullptr);
    ~GroupManageWindow();
    void refresh();

private slots:
    void on_pushButtonCreateGroup_clicked();
    void onCellDoubleClicked(const QModelIndex &index);

private:
    Ui::GroupManageWindow *ui;
    bool checkMutex(int groupId, QString functionName, QString functionDescription);

protected:
    // 重载焦点进入事件
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // GROUPMANAGEWINDOW_H
