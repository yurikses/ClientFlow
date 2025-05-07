#include "clientwindow.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QMessageBox>

ClientWindow::ClientWindow(const QString &tableName, Database *db, QWidget *parent)
    : QDialog(parent), tableName(tableName), database(db), clientId(-1)
{
    setupUI();
}

ClientWindow::ClientWindow(int id, const QString &tableName, Database *db, QWidget *parent)
    : QDialog(parent), tableName(tableName), database(db), clientId(id)
{
    setupUI();
    populateFields(id);
}

void ClientWindow::setupUI()
{
    setWindowTitle(clientId == -1 ? "Создание записи" : "Редактирование записи");

    QFormLayout *formLayout = new QFormLayout;

    Config &config = Config::instance();
    TableConfig tableConfig = config.getTableConfig(tableName);

    for (const FieldConfig &field : tableConfig.fieldConfigs) {
        if (field.tableDesc.isEmpty()) continue;
        if (field.name.toLower() == "id") continue;

        QWidget *widget = nullptr;
        QString fieldType = field.format.toUpper();

        if (fieldType == "TEXT" || fieldType == "VARCHAR") {
            QLineEdit *lineEdit = new QLineEdit(this);
            widget = lineEdit;
        } else if (fieldType == "DATE") {
            QDateEdit *dateEdit = new QDateEdit(this);
            dateEdit->setCalendarPopup(true);
            dateEdit->setDisplayFormat("yyyy-MM-dd");
            widget = dateEdit;
        } else if (fieldType == "INTEGER" || fieldType == "INT") {
            QLineEdit *lineEdit = new QLineEdit(this);
            widget = lineEdit;
        } else if (fieldType == "BOOLEAN") {
            QComboBox *comboBox = new QComboBox(this);
            comboBox->addItems({"false", "true"});
            widget = comboBox;
        }

        if (widget) {
            formLayout->addRow(field.tableDesc + ":", widget);
            fieldWidgets[field.name] = widget;
        }
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ClientWindow::saveClient);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ClientWindow::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

void ClientWindow::populateFields(int id)
{
    QSqlQuery query(database->getDb());
    query.prepare(QString("SELECT * FROM %1 WHERE id = :id").arg(tableName));
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные.");
        reject();
        return;
    }

    for (auto it = fieldWidgets.begin(); it != fieldWidgets.end(); ++it) {
        QString fieldName = it.key();
        QWidget *widget = it.value();

        QVariant value = query.value(fieldName);

        if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->setText(value.toString());
        } else if (QDateEdit *dateEdit = qobject_cast<QDateEdit*>(widget)) {
            dateEdit->setDate(value.toDate());
        } else if (QComboBox *comboBox = qobject_cast<QComboBox*>(widget)) {
            comboBox->setCurrentText(value.toString().toLower() == "true" ? "true" : "false");
        }
    }
}

void ClientWindow::saveClient()
{
    QVariantMap recordData;

    for (auto it = fieldWidgets.begin(); it != fieldWidgets.end(); ++it) {
        QString fieldName = it.key();
        QWidget *widget = it.value();

        QVariant value;

        if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget)) {
            value = lineEdit->text();
        } else if (QDateEdit *dateEdit = qobject_cast<QDateEdit*>(widget)) {
            value = dateEdit->date();
        } else if (QComboBox *comboBox = qobject_cast<QComboBox*>(widget)) {
            value = comboBox->currentText();
        }

        recordData[fieldName] = value;
    }

    database->openConnection("clients.db");

    if (clientId == -1) {
        if (database->insertRecord(tableName, recordData)) {

            accept();
            close();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать запись.");
        }
    } else {
        if (database->updateRecord(tableName, clientId, recordData)) {

            accept();
            close();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить запись.");
        }
    }
}
