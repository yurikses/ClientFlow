#include "clientwindow.h"
#include "validation/OnlyNumsRule.h"
#include "validation/OnlySymbolsRule.h"
#include "validation/emailrule.h"
#include "validation/minlengthrule.h"
#include "validation/notemptyrule.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QMessageBox>
#include <qpushbutton.h>

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

            // Установка правил валидации
            for (const auto &rulePair : field.validationRules) {
                QString ruleType = rulePair.first;
                QVariant ruleValue = rulePair.second;

                ValidationRule* rule = nullptr;

                if (ruleType == "notEmpty") {
                    rule = new NotEmptyRule();
                } else if (ruleType == "email") {
                    rule = new EmailRule();
                } else if (ruleType == "minLength") {
                    bool ok;
                    int minLength = ruleValue.toInt(&ok);
                    if (ok && minLength > 0) {
                        rule = new MinLengthRule(minLength);
                    }
                }else if (ruleType == "onlySymbols") {
                    rule = new OnlySymbolsRule();
                }

                else if (ruleType == "onlyNums") {
                    rule = new OnlyNumsRule();
                }
                if (rule) {
                    fieldValidators[field.name].push_back(rule);
                }
            }
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

bool ClientWindow::validateFields()
{
    Config &config = Config::instance();

    for (auto it = fieldValidators.begin(); it != fieldValidators.end(); ++it) {
        QString fieldName = it.key();
        QList<ValidationRule*> rules = it.value();

        QWidget *widget = fieldWidgets.value(fieldName);
        if (!widget) continue;

        QString value;

        if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget)) {
            value = lineEdit->text().trimmed();
        } else if (QDateEdit *dateEdit = qobject_cast<QDateEdit*>(widget)) {
            value = dateEdit->date().toString(Qt::ISODate);
        } else if (QComboBox *comboBox = qobject_cast<QComboBox*>(widget)) {
            value = comboBox->currentText();
        }

        foreach (ValidationRule* rule , rules){
            if (!rule->validate(value)) {
                qDebug() << fieldName << rule->getDescription()  << rule->validate(value);
                QMessageBox::warning(this, "Ошибка валидации" ,config.getFildDescByName(fieldName)+": " + rule->getDescription());
                if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget)) {
                    lineEdit->setFocus();
                }
                return false;
            }
        }

    }

    return true;
}

void ClientWindow::saveClient()
{
    if (!validateFields()) {
        return;
    }

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
