#ifndef VERSIONMANAGEWINDOW_H
#define VERSIONMANAGEWINDOW_H

#include <QDialog>

namespace Ui {
class VersionManageWindow;
}

class VersionManageWindow : public QDialog
{
    Q_OBJECT

public:
    explicit VersionManageWindow(QWidget *parent = nullptr);
    ~VersionManageWindow();
    void refresh();
    bool isTotalWeightsOver1 () { return totalWeightsOver1; }

private slots:
    void on_pushButtonCreateVersion_clicked();
    void onCellDoubleClicked(const QModelIndex &index);

private:
    Ui::VersionManageWindow *ui;
    bool eventFilter(QObject *watched, QEvent *event) override;

    int rowOffset = 3; // 行偏移量: 分组描述，是否平均权重，权重总和
    int colOffset = 2; // 列偏移量: 分组ID，组描述
    bool totalWeightsOver1;
};

#endif // VERSIONMANAGEWINDOW_H
