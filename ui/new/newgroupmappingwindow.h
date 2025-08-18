#ifndef NEWGROUPMAPPINGWINDOW_H
#define NEWGROUPMAPPINGWINDOW_H

#include <QDialog>

namespace Ui {
class NewGroupMappingWindow;
}

class NewGroupMappingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NewGroupMappingWindow(QWidget *parent = nullptr);
    ~NewGroupMappingWindow();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::NewGroupMappingWindow *ui;
};

#endif // NEWGROUPMAPPINGWINDOW_H
