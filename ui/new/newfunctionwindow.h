#ifndef NEWFUNCTIONWINDOW_H
#define NEWFUNCTIONWINDOW_H

#include <QDialog>
#include "functiondatamanager.h"
namespace Ui {
class newFunctionWindow;
}

class newFunctionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit newFunctionWindow(QWidget *parent = nullptr);
    ~newFunctionWindow();

    FunctionData getFunctionData();

private slots:
    // 确定按钮点击事件
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void onReturnTypeChanged(int index);

private:
    Ui::newFunctionWindow *ui;
};

#endif // NEWFUNCTIONWINDOW_H
