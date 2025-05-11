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
    currentValidationRules = config.getValidationRulesConfig();

    // Настройка таблицы валидации
    QStandardItemModel *validationModel = new QStandardItemModel(this);
    validationModel->setHorizontalHeaderLabels({"Поле", "Правила"});
    ui->validationRulesTableView->setModel(validationModel);

    updateValidationRulesTable(); // заполнение данными
    connect(ui->validationRulesTableView, &QTableView::doubleClicked, this, &settingsdialog::on_validationRulesTableView_doubleClicked);
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
    // 1. Обновление структуры таблицы clients (без валидации)
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

        QJsonObject columnObj;
        columnObj["size"] = size;
        columnObj["format"] = format;
        columnObj["defaultValue"] = "";
        columnObj["tableDesc"] = tableDesc;

        QJsonObject columnEntry;
        columnEntry[columnName] = columnObj;
        columnsArray.append(columnEntry);
    }

    dbObj[tableName] = columnsArray;

    // 2. Сохранение правил валидации в отдельном объекте
    QJsonObject validationRulesObj;

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *nameItem = ui->tableWidget->item(row, 0);
        QString columnName = nameItem ? nameItem->text() : "";

        ValidationRuleEditor *validatorEditor = static_cast<ValidationRuleEditor*>(ui->tableWidget->cellWidget(row, 4));
        QList<ValidationRule*> validationRules = validatorEditor->getRules();

        if (!validationRules.isEmpty()) {
            QJsonArray validationArray;
            for (auto rule : validationRules) {
                QJsonObject ruleObj;
                ruleObj["type"] = rule->getType();
                if (rule->getType() == "minLength") {
                    ruleObj["value"] = rule->getValue().toInt();
                }
                validationArray.append(ruleObj);
            }
            validationRulesObj[columnName] = validationArray;
        }
    }

    // 3. Объединение структуры базы и валидации в один объект
    QJsonObject finalConfig;
    finalConfig["db"] = dbObj;
    finalConfig["validationRules"] = validationRulesObj;

    config.saveConfigFile(finalConfig);
    qDebug() << "Конфигурация успешно обновлена.";

    // 4. Обновляем текущую конфигурацию в экземпляре Config
    config.importJsonFile();
}
void settingsdialog::updateValidationRulesTable() {
    validationModel->clear();
    validationModel->setHorizontalHeaderLabels({"Поле", "Правила"});

    for (auto it = currentValidationRules.begin(); it != currentValidationRules.end(); ++it) {
        QString field = it.key();
        QJsonArray rules = it.value().toArray();

        QStringList ruleDescriptions;
        for (const QJsonValue& ruleVal : rules) {
            QJsonObject ruleObj = ruleVal.toObject();
            QString type = ruleObj.value("type").toString();
            if (type == "minLength") {
                int length = ruleObj.value("value").toInt();
                ruleDescriptions << QString("%1 (%2)").arg(type).arg(length);
            } else {
                ruleDescriptions << type;
            }
        }

        QStandardItem* fieldItem = new QStandardItem(field);
        fieldItem->setEditable(false);

        QStandardItem* rulesItem = new QStandardItem(ruleDescriptions.join(", "));
        rulesItem->setEditable(false);

        validationModel->appendRow({fieldItem, rulesItem});
    }
}

void settingsdialog::on_addValidationRuleButton_clicked() {
    Config &config = Config::instance();
    QStringList allFields = config.getFieldNamesForTable();
    ValidationRuleDialog dialog(allFields);
    if (dialog.exec() == QDialog::Accepted) {
        QString field = dialog.getSelectedField();
        QList<ValidationRule*> rules = dialog.getRules();

        QJsonArray ruleArray;
        for (auto rule : rules) {
            QJsonObject ruleObj;
            ruleObj["type"] = rule->getType();
            if (rule->getType() == "minLength")
                ruleObj["value"] = rule->getValue().toInt();
            ruleArray.append(ruleObj);
        }

        currentValidationRules[field] = ruleArray;
        updateValidationRulesTable();
    }
}

void settingsdialog::on_removeValidationRuleButton_clicked() {
    QModelIndex index = ui->validationRulesTableView->currentIndex();
    if (!index.isValid())
        return;

    QString field = validationModel->item(index.row(), 0)->text();
    currentValidationRules.remove(field); // Удаляем запись для этого поля
    updateValidationRulesTable();
}

void settingsdialog::on_validationRulesTableView_doubleClicked(const QModelIndex& index) {
    if (!index.isValid())
        return;

    QString field = validationModel->item(index.row(), 0)->text();
    QJsonArray existingRules = currentValidationRules.value(field).toArray();

    QStringList ruleStrings;
    QList<ValidationRule*> rules;
    for (const QJsonValue& val : existingRules) {
        QJsonObject obj = val.toObject();
        QString type = obj.value("type").toString();
        if (type == "notEmpty") {
            rules.append(new NotEmptyRule());
        } else if (type == "email") {
            rules.append(new EmailRule());
        } else if (type == "minLength") {
            rules.append(new MinLengthRule(obj.value("value").toInt()));
        }
    }
    Config &config = Config::instance();
    QStringList allFields = config.getFieldNamesForTable();
    ValidationRuleDialog dialog(allFields, field, rules);
    if (dialog.exec() == QDialog::Accepted) {
        QJsonArray updatedRules;
        for (auto rule : dialog.getRules()) {
            QJsonObject ruleObj;
            ruleObj["type"] = rule->getType();
            if (rule->getType() == "minLength")
                ruleObj["value"] = rule->getValue().toInt();
            updatedRules.append(ruleObj);
        }
        currentValidationRules[field] = updatedRules;
        updateValidationRulesTable();
    }
}
