#ifndef NEWVERSIONWINDOW_H
#define NEWVERSIONWINDOW_H

#include <QDialog>

namespace Ui {
class newVersionWindow;
}

class newVersionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit newVersionWindow(QWidget *parent = nullptr);
    ~newVersionWindow();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::newVersionWindow *ui;  // UI 界面对象
};

#endif // NEWVERSIONWINDOW_H
