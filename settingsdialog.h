#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <qstandarditemmodel.h>
#include <qtablewidget>
#include "config.h"
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
    void on_validationRulesTableWidget_cellDoubleClicked(int row, int column);

private:
    Ui::settingsdialog *ui;
    QJsonObject currentValidationRules;
    QStandardItemModel *validationModel;
    Config &config;

    void updateValidationRulesTable();

    // Метод для получения текущих правил валидации из таблицы
    QList<ValidationRule*> getValidationRules() const;

    // Метод для установки правил валидации в таблицу
    void setValidationRules(const QList<ValidationRule*>& rules);
    void saveValidationRulesToConfig();
    // Метод для сохранения правил валидации в JSON
    void saveValidationRules(QJsonObject& dbObj) const;
};

#endif // SETTINGSDIALOG_H
