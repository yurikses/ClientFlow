#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "database.h"
#include "config.h"
#include <QComboBox>
#include <QMessageBox>
#include <QJsonArray>
settingsdialog::settingsdialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settingsdialog)
{
    Config &config = Config::instance();
    TableConfig clientTableConfig = config.getTableConfig("clients");

    ui->setupUi(this);

    // Устанавливаем количество строк и столбцов
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setRowCount(clientTableConfig.fieldConfigs.size());

    // Заголовки таблицы (на русском)
    QStringList headers = {"Название поля", "Размер поля", "Тип поля", "Описание поля"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Заполняем таблицу данными из конфигурации
    for (int row = 0; row < clientTableConfig.fieldConfigs.size(); ++row) {
        const FieldConfig &field = clientTableConfig.fieldConfigs[row];

        // Название поля
        QTableWidgetItem *nameItem = new QTableWidgetItem(field.name);
        ui->tableWidget->setItem(row, 0, nameItem);

        // Если это "id", блокируем редактирование
        if (field.name == "id") {
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable); // Запрещаем редактирование
        }

        // Размер поля
        QTableWidgetItem *sizeItem = new QTableWidgetItem(QString::number(field.size));
        sizeItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, 1, sizeItem);

        // Если это "id", также запрещаем изменение размера
        if (field.name == "id") {
            sizeItem->setFlags(sizeItem->flags() & ~Qt::ItemIsEditable);
        }

        // Тип поля — комбобокс
        QComboBox *typeBox = new QComboBox();
        typeBox->addItems({"INTEGER", "INT", "TEXT", "DATE", "BOOLEAN"});
        typeBox->setCurrentText(field.format);

        // Если это "id", запрещаем выбор других вариантов кроме "INTEGER"
        if (field.name == "id") {
            typeBox->setEnabled(false); // Полностью блокируем выбор
            typeBox->setCurrentText("INTEGER"); // Зафиксируем значение
        }

        ui->tableWidget->setCellWidget(row, 2, typeBox);

        // Описание поля
        QTableWidgetItem *descItem = new QTableWidgetItem(field.tableDesc);
        ui->tableWidget->setItem(row, 3, descItem);

        // Также запрещаем редактирование описания для id
        if (field.name == "id") {
            descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);
        }
    }
}

settingsdialog::~settingsdialog()
{
    delete ui;
}

void settingsdialog::on_addNewFieldButton_clicked()
{
    // Получаем текущее количество строк в таблице
    int rowCount = ui->tableWidget->rowCount();

    // Добавляем новую строку в таблицу
    ui->tableWidget->insertRow(rowCount);

    // Название поля (пустое по умолчанию)
    QTableWidgetItem *nameItem = new QTableWidgetItem("Новое поле");
    ui->tableWidget->setItem(rowCount, 0, nameItem);

    // Размер поля (по умолчанию 10)
    QTableWidgetItem *sizeItem = new QTableWidgetItem("10");
    sizeItem->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(rowCount, 1, sizeItem);

    // Тип поля — комбобокс
    QComboBox *typeBox = new QComboBox();
    typeBox->addItems({"INT", "TEXT", "DATE", "BOOLEAN"});
    typeBox->setCurrentIndex(0); // INT по умолчанию
    ui->tableWidget->setCellWidget(rowCount, 2, typeBox);

    // Описание поля (пустое по умолчанию)
    QTableWidgetItem *descItem = new QTableWidgetItem("");
    ui->tableWidget->setItem(rowCount, 3, descItem);

    // Растягиваем все колонки
    for (int col = 0; col < 4; ++col) {
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }


}


void settingsdialog::on_delFieldButton_clicked()
{

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

        // Получаем оригинальное имя из предыдущей конфигурации
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

        // Формируем JSON объект
        QJsonObject columnObj;
        columnObj["size"] = size;
        columnObj["format"] = format;
        columnObj["defaultValue"] = "";
        columnObj["tableDesc"] = tableDesc;

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
    QJsonObject confObj;
    confObj["db"] = dbObj;
    config.saveConfigFile(confObj);

    qDebug() << "Конфигурация успешно обновлена.";
}
