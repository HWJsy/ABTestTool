#ifndef GROUPUPDATEMAPPINGWINDOW_H
#define GROUPUPDATEMAPPINGWINDOW_H

#include <QDialog>

namespace Ui {
class GroupUpdateMappingWindow;
}

class GroupUpdateMappingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GroupUpdateMappingWindow(QWidget *parent = nullptr);
    ~GroupUpdateMappingWindow();
    void refresh();

private slots:
    void on_pushButtonCreateMapping_clicked();  // 新建映射按钮点击
    void onCellDoubleClicked(const QModelIndex &index);
    void refreshAfterMapping();  // 刷新tableView数据

private:
    bool eventFilter(QObject *watched, QEvent *event);
    Ui::GroupUpdateMappingWindow *ui;
};

#endif // GROUPUPDATEMAPPINGWINDOW_H
