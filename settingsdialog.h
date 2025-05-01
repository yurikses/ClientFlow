#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class settingsdialog;
}

class settingsdialog : public QDialog
{
    Q_OBJECT

public:
    explicit settingsdialog(QWidget *parent = nullptr);
    ~settingsdialog();

private slots:
    void on_addNewFieldButton_clicked();

    void on_delFieldButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void updateConfigFile();

private:
    Ui::settingsdialog *ui;
};

#endif // SETTINGSDIALOG_H
