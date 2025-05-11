#include "settingsdialog.h"
#include "config.h"
#include "ui_settingsdialog.h"
#include "validation/emailrule.h"
#include "validation/minlengthrule.h"
#include "validation/notemptyrule.h"
#include "validationruleeditor.h"
#include "validationruledialog.h"
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>
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
    QStringList headers = {"Название поля", "Размер поля", "Тип поля", "Описание поля"};
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

    }

    // Растягиваем все колонки
    for (int col = 0; col < 5; ++col) {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }

    // Загрузка правил валидации из конфига
    currentValidationRules = config.getValidationRules();


    updateValidationRulesTable(); // заполнение данными
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

void settingsdialog::updateConfigFile() {
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

        // Формируем JSON объект
        QJsonObject columnObj;
        columnObj["size"] = size;
        columnObj["format"] = format;
        columnObj["defaultValue"] = "";
        columnObj["tableDesc"] = tableDesc;

        // Добавляем валидацию
        if (currentValidationRules.contains(columnName)) {
            columnObj["validation"] = currentValidationRules.value(columnName).toArray();
        }

        QJsonObject columnEntry;
        columnEntry[columnName] = columnObj;
        columnsArray.append(columnEntry);
    }

    dbObj[tableName] = columnsArray;
    config.saveConfigFile(dbObj);
    qDebug() << "Конфигурация успешно обновлена.";
}


void settingsdialog::updateValidationRulesTable() {
    ui->validationRulesTableWidget->clear();
    ui->validationRulesTableWidget->setRowCount(0);
    ui->validationRulesTableWidget->setColumnCount(2);
    ui->validationRulesTableWidget->setHorizontalHeaderLabels({"Поле", "Правила"});

    // Получаем список всех полей из основной таблицы
    QList<QString> fieldNames;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem* nameItem = ui->tableWidget->item(row, 0);
        if (nameItem) {
            fieldNames << nameItem->text();
        }
    }

    // Обновляем правила для всех полей
    for (const QString& field : fieldNames) {
        int row = ui->validationRulesTableWidget->rowCount();
        ui->validationRulesTableWidget->insertRow(row);

        // Получаем правила из currentValidationRules
        QJsonArray rules = currentValidationRules.value(field).toArray();

        // Формируем строку с описанием правил
        QStringList ruleDescriptions;
        for (const QJsonValue& ruleVal : rules) {
            QJsonObject ruleObj = ruleVal.toObject();
            QString type = ruleObj.value("type").toString();
            if (type == "notEmpty") {
                ruleDescriptions << "Не пустое";
            } else if (type == "email") {
                ruleDescriptions << "Email";
            } else if (type == "minLength") {
                int length = ruleObj.value("value").toInt();
                ruleDescriptions << QString("Мин. длина %1").arg(length);
            }
        }

        // Устанавливаем значения в таблицу
        QTableWidgetItem* fieldItem = new QTableWidgetItem(field);
        QTableWidgetItem* rulesItem = new QTableWidgetItem(ruleDescriptions.join(", "));
        fieldItem->setFlags(fieldItem->flags() & ~Qt::ItemIsEditable); // только чтение
        rulesItem->setFlags(rulesItem->flags() & ~Qt::ItemIsEditable);

        ui->validationRulesTableWidget->setItem(row, 0, fieldItem);
        ui->validationRulesTableWidget->setItem(row, 1, rulesItem);
    }

    // Растягиваем колонки
    for (int col = 0; col < 2; ++col) {
        ui->validationRulesTableWidget->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }
}

void settingsdialog::on_addValidationRuleButton_clicked() {
    QStringList allFields = Config::instance().getFieldNamesForTable();
    ValidationRuleDialog dialog(allFields);

    if (dialog.exec() == QDialog::Accepted) {
        QString field = dialog.getSelectedField();
        QList<ValidationRule*> rules = dialog.getRules();

        // Сохраняем временно в currentValidationRules
        QJsonArray ruleArray;
        for (auto rule : rules) {
            QJsonObject ruleObj;
            ruleObj["type"] = rule->getType();
            if (rule->getType() == "minLength") {
                ruleObj["value"] = rule->getValue().toInt();
            }
            ruleArray.append(ruleObj);
        }
        currentValidationRules[field] = ruleArray;
        updateValidationRulesTable();
    }
}

void settingsdialog::on_removeValidationRuleButton_clicked() {
    QModelIndex index = ui->validationRulesTableWidget->currentIndex();
    if (!index.isValid()) return;

    QString field = ui->validationRulesTableWidget->item(index.row(), 0)->text();
    currentValidationRules.remove(field); // Удаляем правило
    updateValidationRulesTable();
}

void settingsdialog::saveValidationRulesToConfig() {
    QJsonObject dbObj = Config::instance().getBDConfig().value("db").toObject();
    QJsonArray columnsArray = dbObj.value("clients").toArray();

    for (int i = 0; i < columnsArray.size(); ++i) {
        QJsonObject columnEntry = columnsArray[i].toObject();
        for (const QString& columnName : columnEntry.keys()) {
            QJsonObject columnDetails = columnEntry.value(columnName).toObject();

            // Удаляем старые правила
            columnDetails.remove("validation");

            // Добавляем новые из currentValidationRules
            if (currentValidationRules.contains(columnName)) {
                columnDetails["validation"] = currentValidationRules[columnName].toArray();
            }

            columnEntry[columnName] = columnDetails;
            columnsArray[i] = columnEntry;
        }
    }

    dbObj["clients"] = columnsArray;
    Config::instance().saveConfigFile(dbObj);
    qDebug() << "Правила валидации сохранены в файл.";
}


void settingsdialog::on_validationRulesTableWidget_cellDoubleClicked(int row, int) {
    QString field = ui->validationRulesTableWidget->item(row, 0)->text();
    QJsonArray ruleArray = currentValidationRules.value(field).toArray();

    // Преобразуем в список правил
    QList<ValidationRule*> existingRules;
    for (const QJsonValue& val : ruleArray) {
        QJsonObject obj = val.toObject();
        QString type = obj.value("type").toString();
        if (type == "notEmpty") {
            existingRules.append(new NotEmptyRule());
        } else if (type == "email") {
            existingRules.append(new EmailRule());
        } else if (type == "minLength") {
            existingRules.append(new MinLengthRule(obj.value("value").toInt()));
        }
    }

    // Открываем диалог редактирования
    ValidationRuleDialog dialog(Config::instance().getFieldNamesForTable(), field, existingRules);
    if (dialog.exec() == QDialog::Accepted) {
        QJsonArray updatedRules;
        for (auto rule : dialog.getRules()) {
            QJsonObject ruleObj;
            ruleObj["type"] = rule->getType();
            if (rule->getType() == "minLength") {
                ruleObj["value"] = rule->getValue().toInt();
            }
            updatedRules.append(ruleObj);
        }
        currentValidationRules[field] = updatedRules;
        updateValidationRulesTable();
    }
}

