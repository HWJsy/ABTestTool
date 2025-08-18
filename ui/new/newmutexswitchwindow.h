#ifndef NEWMUTEXSWITCHWINDOW_H
#define NEWMUTEXSWITCHWINDOW_H

#include <QDialog>

namespace Ui {
class NewMutexSwitchWindow;
}

class NewMutexSwitchWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NewMutexSwitchWindow(QWidget *parent = nullptr);
    ~NewMutexSwitchWindow();

private slots:
    void on_pushButtonCreateMutexSwitch_clicked();
    void onCellDoubleClicked(const QModelIndex &index);

private:
    Ui::NewMutexSwitchWindow *ui;
    void refresh();
};

#endif // NEWMUTEXSWITCHWINDOW_H
