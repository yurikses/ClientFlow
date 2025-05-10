#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include "validation/validationrule.h"
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
    void on_pushButton_2_clicked(); // Отмена
    void on_pushButton_clicked();   // Сохранить
    void updateConfigFile();

private:
    Ui::settingsdialog *ui;

    // Метод для получения текущих правил валидации из таблицы
    QList<ValidationRule*> getValidationRules() const;

    // Метод для установки правил валидации в таблицу
    void setValidationRules(const QList<ValidationRule*>& rules);

    // Метод для сохранения правил валидации в JSON
    void saveValidationRules(QJsonObject& dbObj) const;
};

#endif // SETTINGSDIALOG_H
