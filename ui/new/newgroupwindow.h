#ifndef NEWGROUPWINDOW_H
#define NEWGROUPWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDialogButtonBox>

namespace Ui {
class newGroupWindow;
}

class newGroupWindow : public QDialog
{
    Q_OBJECT

public:
    explicit newGroupWindow(QWidget *parent = nullptr);
    ~newGroupWindow();

signals:
    void groupDescriptionEntered(const QString& groupDescription);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::newGroupWindow *ui;
    QLabel *label;
    QLineEdit *lineEditGroupDescription;
    QDialogButtonBox *buttonBox;
    bool checkMutex();
};

#endif // NEWGROUPWINDOW_H
