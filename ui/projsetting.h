#ifndef PROJSETTING_H
#define PROJSETTING_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
namespace Ui {
class ProjSetting;
}

class ProjSetting : public QDialog
{
    Q_OBJECT

public:
    explicit ProjSetting(QWidget *parent = nullptr);
    ~ProjSetting();

private slots:
    void on_pushButtonCppExportPath_clicked();

private:
    Ui::ProjSetting *ui;
};

#endif // PROJSETTING_H
