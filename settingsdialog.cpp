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
    , ui(new Ui::settingsdialog), config(Config::instance())
{
    ui->setupUi(this);
    TableConfig clientTableConfig = config.getTableConfig("clients");

    // Устанавливаем количество строк и столбцов
    ui->tableWidget->setColumnCount(4); // Добавляем столбец для валидации
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
    for (int col = 0; col < 4; ++col) {
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


    // Растягиваем все колонки
    for (int col = 0; col < 4; ++col) {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }
}

void settingsdialog::on_delFieldButton_clicked()
{
    // Получаем индекс текущей выбранной строки
    int currentRow = ui->tableWidget->currentRow();

    // Проверяем, что строка выбрана
    if (currentRow < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите поле для удаления.");
        return;
    }

    // Запрашиваем подтверждение удаления
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Подтверждение",
        "Вы действительно хотите удалить это поле?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        // Удаляем строку из таблицы
        ui->tableWidget->removeRow(currentRow);
    }
}

void settingsdialog::on_pushButton_2_clicked()
{
    reject();
}

void settingsdialog::on_pushButton_clicked()
{
    updateConfigFile();
    // saveValidationRulesToConfig();
    QMessageBox::information(this, "Успех", "Настройки сохранены.");
    accept();
}

void settingsdialog::updateConfigFile() {
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
        QString originalName = config.getFieldNamesForTable().value(row); // исходное имя до изменений
        QStringList existingOldNames;

        // Получаем существующие oldNames из конфига для этого поля
        const FieldConfig &existingField = config.getTableConfig("clients").fieldConfigs.value(row);
        if (!existingField.name.isEmpty()) {
            existingOldNames = existingField.oldNames;
        }

        // Проверяем, было ли поле переименовано
        QStringList newOldNames = existingOldNames;

        if (!originalName.isEmpty() && !columnName.isEmpty()) {
            if (originalName != columnName) {
                // Добавляем только если новое имя отличается от оригинала
                if (!newOldNames.contains(originalName)) {
                    newOldNames.append(originalName);
                }
            } else {
                // Если вернули старое имя — не добавляем его в oldNames снова
                // но сохраняем существующую историю
            }
        }

        QJsonObject columnObj;
        columnObj["size"] = size;
        columnObj["format"] = format;
        columnObj["defaultValue"] = "";
        columnObj["tableDesc"] = tableDesc;

        // Сохраняем правила валидации
        QJsonArray validationRules = config.getFieldValidation(columnName);
        if (!validationRules.isEmpty()) {
            columnObj["validation"] = validationRules;
        }

        if (!newOldNames.isEmpty()) {
            QJsonArray oldNamesArray;
            for (const QString &name : newOldNames) {
                oldNamesArray.append(name);
            }
            columnObj["oldNames"] = oldNamesArray;
        }


        QJsonObject columnEntry;
        columnEntry[columnName] = columnObj;
        columnsArray.append(columnEntry);
    }

    dbObj[tableName] = columnsArray;
    QJsonObject jsonFile;
    jsonFile["db"] = dbObj;
    config.saveConfigFile(jsonFile);
}


void settingsdialog::updateValidationRulesTable() {
    ui->validationRulesTableWidget->clear();
    ui->validationRulesTableWidget->setRowCount(0);
    ui->validationRulesTableWidget->setColumnCount(2);
    ui->validationRulesTableWidget->setHorizontalHeaderLabels({"Поле", "Правила"});

    QStringList allFields = config.getFieldNamesForTable();
    for (const QString& field : allFields) {
        int row = ui->validationRulesTableWidget->rowCount();
        ui->validationRulesTableWidget->insertRow(row);

        // Получаем правила из конфига
        QJsonArray validationObj = config.getFieldValidation(field);
        QJsonArray validationArray = validationObj.isEmpty() ? QJsonArray() :validationObj;

        // Формируем строку с описанием правил
        QStringList ruleDescriptions;
        for (const QJsonValue& val : validationArray) {
            QJsonObject rule = val.toObject();
            QString type = rule.value("type").toString();
            if (type == "notEmpty") {
                ruleDescriptions << "Не может быть пустым";
            } else if (type == "email") {
                ruleDescriptions << "Email";
            } else if (type == "minLength") {
                int length = rule.value("value").toInt();
                ruleDescriptions << QString("Минимальная длина %1").arg(length);
            } else if (type == "onlySymbols") {
                ruleDescriptions << "Только буквы";
            } else if (type == "onlyNums") {
                ruleDescriptions << "Только цифры";
            }

        }

        QTableWidgetItem* fieldItem = new QTableWidgetItem(field);
        QTableWidgetItem* rulesItem = new QTableWidgetItem(ruleDescriptions.join(", "));
        fieldItem->setFlags(fieldItem->flags() & ~Qt::ItemIsEditable); // только чтение
        rulesItem->setFlags(rulesItem->flags() & ~Qt::ItemIsEditable);

        ui->validationRulesTableWidget->setItem(row, 0, fieldItem);
        ui->validationRulesTableWidget->setItem(row, 1, rulesItem);
    }

    // Растягиваем таблицу
    for (int col = 0; col < 2; ++col) {
        ui->validationRulesTableWidget->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }
}



// void settingsdialog::saveValidationRulesToConfig() {
//     QJsonObject dbObj = config.getBDConfig();
//     QJsonArray columnsArray = dbObj.value("clients").toArray();

//     for (int i = 0; i < columnsArray.size(); ++i) {
//         QJsonObject columnEntry = columnsArray[i].toObject();
//         for (const QString& columnName : columnEntry.keys()) {
//             QJsonObject columnDetails = columnEntry.value(columnName).toObject();

//             // Удаляем старые правила
//             columnDetails.remove("validation");

//             // Добавляем новые из currentValidationRules
//             if (currentValidationRules.contains(columnName)) {
//                 columnDetails["validation"] = currentValidationRules[columnName].toArray();
//             }

//             columnEntry[columnName] = columnDetails;
//             columnsArray[i] = columnEntry;
//         }
//     }

//     dbObj["clients"] = columnsArray;
//     QJsonObject jsonFile;
//     jsonFile["db"] = dbObj;
//     config.saveConfigFile(jsonFile);
//     qDebug() << "Правила валидации сохранены в файл.";
// }


void settingsdialog::on_validationRulesTableWidget_cellDoubleClicked(int row, int) {
    QString field = ui->validationRulesTableWidget->item(row, 0)->text();
    QJsonArray currentRules = config.getFieldValidation(field);


    ValidationRuleDialog dialog(field, currentRules);
    if (dialog.exec() == QDialog::Accepted) {
        config.setFieldValidation(field, dialog.getRules());
        updateValidationRulesTable(); // обновляем интерфейс
    }
}

