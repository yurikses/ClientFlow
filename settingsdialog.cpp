#include "settingsdialog.h"
#include "config.h"
#include "ui_settingsdialog.h"
#include "validationruleeditor.h"
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>

settingsdialog::settingsdialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settingsdialog)
{
    ui->setupUi(this);

    Config &config = Config::instance();
    TableConfig clientTableConfig = config.getTableConfig("clients");

    // Устанавливаем количество строк и столбцов
    ui->tableWidget->setColumnCount(5); // Добавляем столбец для валидации
    ui->tableWidget->setRowCount(clientTableConfig.fieldConfigs.size());

    // Заголовки таблицы
    QStringList headers = {"Название поля", "Размер поля", "Тип поля", "Описание поля", "Правила валидации"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Заполняем таблицу данными из конфигурации
    for (int row = 0; row < clientTableConfig.fieldConfigs.size(); ++row) {
        const FieldConfig &field = clientTableConfig.fieldConfigs[row];

        // Название поля
        QTableWidgetItem *nameItem = new QTableWidgetItem(field.name);
        ui->tableWidget->setItem(row, 0, nameItem);

        // Размер поля
        QTableWidgetItem *sizeItem = new QTableWidgetItem(QString::number(field.size));
        sizeItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, 1, sizeItem);

        // Тип поля — комбобокс
        QComboBox *typeBox = new QComboBox();
        typeBox->addItems({"INTEGER", "INT", "TEXT", "DATE", "BOOLEAN"});
        typeBox->setCurrentText(field.format);
        ui->tableWidget->setCellWidget(row, 2, typeBox);

        // Описание поля
        QTableWidgetItem *descItem = new QTableWidgetItem(field.tableDesc);
        ui->tableWidget->setItem(row, 3, descItem);

        // Правила валидации
        ValidationRuleEditor *validatorEditor = new ValidationRuleEditor();
        validatorEditor->setRules(config.getValidationRulesForField(field.name));
        ui->tableWidget->setCellWidget(row, 4, validatorEditor);
    }

    // Растягиваем все колонки
    for (int col = 0; col < 5; ++col) {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }
}

settingsdialog::~settingsdialog()
{
    delete ui;
}

void settingsdialog::on_addNewFieldButton_clicked()
{
    int rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(rowCount);

    // Название поля
    QTableWidgetItem *nameItem = new QTableWidgetItem("Новое поле");
    ui->tableWidget->setItem(rowCount, 0, nameItem);

    // Размер поля
    QTableWidgetItem *sizeItem = new QTableWidgetItem("10");
    sizeItem->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(rowCount, 1, sizeItem);

    // Тип поля — комбобокс
    QComboBox *typeBox = new QComboBox();
    typeBox->addItems({"INT", "TEXT", "DATE", "BOOLEAN"});
    typeBox->setCurrentIndex(0); // INT по умолчанию
    ui->tableWidget->setCellWidget(rowCount, 2, typeBox);

    // Описание поля
    QTableWidgetItem *descItem = new QTableWidgetItem("");
    ui->tableWidget->setItem(rowCount, 3, descItem);

    // Правила валидации
    ValidationRuleEditor *validatorEditor = new ValidationRuleEditor();
    ui->tableWidget->setCellWidget(rowCount, 4, validatorEditor);

    // Растягиваем все колонки
    for (int col = 0; col < 5; ++col) {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }
}

void settingsdialog::on_delFieldButton_clicked()
{
    // Реализуйте удаление выбранной строки
}

void settingsdialog::on_pushButton_2_clicked()
{
    reject();
}

void settingsdialog::on_pushButton_clicked()
{
    updateConfigFile();
    QMessageBox::information(this, "Успех", "Настройки сохранены.");
    accept();
}

void settingsdialog::updateConfigFile()
{
    Config &config = Config::instance();
    QJsonObject dbObj = config.getBDConfig().value("db").toObject();
    QString tableName = "clients";

    dbObj.remove(tableName);
    QJsonArray columnsArray;

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *nameItem = ui->tableWidget->item(row, 0);
        QString columnName = nameItem ? nameItem->text() : "";

        bool ok;
        int size = ui->tableWidget->item(row, 1)->text().toInt(&ok);
        if (!ok) size = 10;

        QString format = static_cast<QComboBox*>(ui->tableWidget->cellWidget(row, 2))->currentText();
        QString tableDesc = ui->tableWidget->item(row, 3) ? ui->tableWidget->item(row, 3)->text() : "";

        // Получаем правила валидации
        ValidationRuleEditor *validatorEditor = static_cast<ValidationRuleEditor*>(ui->tableWidget->cellWidget(row, 4));
        QList<ValidationRule*> validationRules = validatorEditor->getRules();

        // Формируем JSON объект
        QJsonObject columnObj;
        columnObj["size"] = size;
        columnObj["format"] = format;
        columnObj["defaultValue"] = "";
        columnObj["tableDesc"] = tableDesc;

        // Сохраняем правила валидации
        QJsonArray validationArray;
        for (auto rule : validationRules) {
            QJsonObject ruleObj;
            ruleObj["type"] = rule->getType();
            if (rule->getType() == "minLength") {
                ruleObj["value"] = rule->getValue().toInt();
            }
            validationArray.append(ruleObj);
        }
        columnObj["validation"] = validationArray;

        QJsonObject columnEntry;
        columnEntry[columnName] = columnObj;
        columnsArray.append(columnEntry);
    }

    dbObj[tableName] = columnsArray;
    QJsonObject confObj;
    confObj["db"] = dbObj;
    config.saveConfigFile(confObj);

    qDebug() << "Конфигурация успешно обновлена.";
}
