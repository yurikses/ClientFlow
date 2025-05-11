#include "config.h"
#include "validation/emailrule.h"
#include "validation/minlengthrule.h"
#include "validation/notemptyrule.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QMessageBox>
#include <QObject>

struct ColumnSettings {
    QString tableName;
    QString columnName;
    int size;
    QString format;
    QString defaultValue;
    QString tableDesc;
};


Config::Config(QObject *parent) : QObject(parent) {

}

Config& Config::instance()
{
    static Config instance; // Создается один раз
    instance.importJsonFile();
    instance.config = instance.getTableConfig("clients");
    return instance;
}

void Config::importJsonFile(){
    QDir currentDir = QDir::currentPath();
    QString fileName = "config.json";
    QString filePath = currentDir.filePath(fileName);
    QFile file(filePath);

    if (!file.exists()) {
        ColumnSettings dbDefault[] = {
                                      {"clients", "id", 10, "INTEGER", "NULL", "Идентификатор"},
                                      {"clients", "name", 50, "TEXT", "", "ФИО"},
                                      {"clients", "age", 3, "INTEGER", "0", "Возраст"},
                                      {"clients", "phone", 3, "VARCHAR", "", "Номер телефона"},
                                      {"clients", "birthDay", 3, "DATE", "", "День рождения"},
                                      };

        QJsonObject settingsObj;

        for (const auto& setting : dbDefault) {
            QJsonObject columnObj;
            columnObj["size"] = setting.size;
            columnObj["format"] = setting.format;
            columnObj["defaultValue"] = setting.defaultValue;
            columnObj["tableDesc"] = setting.tableDesc;

            QJsonArray columnsArray;
            if (settingsObj.contains(setting.tableName)) {
                columnsArray = settingsObj[setting.tableName].toArray();
            }
            QJsonObject columnEntry;
            columnEntry[setting.columnName] = columnObj;
            columnsArray.append(columnEntry);

            settingsObj[setting.tableName] = columnsArray;
        }

        QJsonObject confObj;
        confObj["db"] = settingsObj;
        saveConfigFile(confObj);

        qDebug() << "JSON файл успешно создан.";
    }

    configFile = loadConfigFile();
}

void Config::saveConfigFile(const QJsonObject &config) {
    QFile file("config.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл для записи.";
        return;
    }

    QJsonDocument jsonDoc(config);
    QTextStream out(&file);
    out << jsonDoc.toJson(QJsonDocument::Indented);
    file.close();
}

QJsonObject Config::loadConfigFile() {
    QFile file("config.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл для чтения.";
        return QJsonObject();
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qDebug() << "Не удалось распарсить JSON или он не является объектом.";
        return QJsonObject();
    }

    return jsonDoc.object();
}


TableConfig Config::getTableConfig(const QString &tableName) {
    TableConfig tableConfig;
    QJsonObject dbObj = configFile.value("db").toObject();

    if (dbObj.contains(tableName)) {
        QJsonArray columnsArray = dbObj.value(tableName).toArray();

        for (const QJsonValue &columnValue : columnsArray) {
            QJsonObject columnEntry = columnValue.toObject();
            for (const QString &columnName : columnEntry.keys()) {
                QJsonObject columnDetails = columnEntry.value(columnName).toObject();

                FieldConfig field;
                field.name = columnName;
                field.format = columnDetails.value("format").toString();
                field.size = columnDetails.value("size").toInt();
                field.defaultValue = columnDetails.value("defaultValue").toString();
                field.tableDesc = columnDetails.value("tableDesc").toString();

                // Загружаем старые имена
                QJsonArray oldNamesArray = columnDetails.value("oldNames").toArray();
                for (const QJsonValue &val : oldNamesArray) {
                    field.oldNames.append(val.toString());
                }

                tableConfig.fieldConfigs.append(field);
            }
        }
    }

    return tableConfig;
}

QList<QString> Config::getBDlist(){
    QJsonObject dbObj = configFile.value("db").toObject();
    QList<QString> BDheaders;
    QJsonArray clientTable = dbObj.value("clients").toArray();
    QString tableDesc;
    for (const QJsonValue &columnValues : clientTable)
    {
        QJsonObject columnObj = columnValues.toObject();
        for(const QString &column : columnObj.keys())
        {
            tableDesc = columnObj.value(column).toObject().value("tableDesc").toString();
        }
        if (!tableDesc.isEmpty()) {
            BDheaders.append(tableDesc);
            continue;
        }
    }
    return BDheaders;
}

QJsonObject Config::getBDConfig(){
    QJsonObject dbObj = configFile.value("db").toObject();
    return dbObj;
}

QList<QString> Config::getFieldNamesForTable() {
    QJsonObject dbObj = configFile.value("db").toObject();
    QList<QString> fieldNames;
    QString tableName = "clients";

    if (dbObj.contains(tableName)) {
        QJsonArray columnsArray = dbObj.value(tableName).toArray();

        for (const QJsonValue &columnEntry : columnsArray) {
            if (columnEntry.isObject()) {
                QJsonObject columnObj = columnEntry.toObject();
                for (const QString &columnName : columnObj.keys()) {
                    QJsonObject columnDetails = columnObj.value(columnName).toObject();
                    QString tableDesc = columnDetails.value("tableDesc").toString();

                    if (!tableDesc.isEmpty()) {
                        fieldNames.append(columnName);
                    }
                }
            }
        }
    }

    return fieldNames;
}

QList<QString> Config::getFieldDescForTable() {
    QJsonObject dbObj = configFile.value("db").toObject();
    QList<QString> fieldNames;
    QString tableName = "clients";

    if (dbObj.contains(tableName)) {
        QJsonArray columnsArray = dbObj.value(tableName).toArray();

        for (const QJsonValue &columnEntry : columnsArray) {
            if (columnEntry.isObject()) {
                QJsonObject columnObj = columnEntry.toObject();
                for (const QString &columnName : columnObj.keys()) {
                    QJsonObject columnDetails = columnObj.value(columnName).toObject();
                    QString tableDesc = columnDetails.value("tableDesc").toString();

                    if (!tableDesc.isEmpty()) {
                        fieldNames.append(tableDesc);
                    }
                }
            }
        }
    }

    return fieldNames;
}

QList<ValidationRule*> Config::getValidationRulesForField(const QString& fieldName) const
{
    QJsonObject dbObj = configFile.value("db").toObject();
    QJsonArray columnsArray = dbObj.value("clients").toArray();

    for (const QJsonValue& columnValue : columnsArray) {
        QJsonObject columnEntry = columnValue.toObject();
        for (const QString& columnName : columnEntry.keys()) {
            if (columnName == fieldName) {
                QJsonArray validationArray = columnEntry.value(fieldName).toObject().value("validation").toArray();
                QList<ValidationRule*> rules;
                for (const QJsonValue& ruleValue : validationArray) {
                    QJsonObject ruleObj = ruleValue.toObject();
                    QString type = ruleObj.value("type").toString();
                    if (type == "notEmpty") {
                        rules.append(new NotEmptyRule());
                    } else if (type == "email") {
                        rules.append(new EmailRule());
                    } else if (type == "minLength") {
                        int value = ruleObj.value("value").toInt();
                        rules.append(new MinLengthRule(value));
                    }
                }
                return rules;
            }
        }
    }
    return {};
}

void Config::saveValidationRules(const QString& fieldName, const QList<ValidationRule*>& rules)
{
    QJsonObject dbObj = configFile.value("db").toObject();
    QJsonArray columnsArray = dbObj.value("clients").toArray();

    for (QJsonValue columnValue : columnsArray) {
        QJsonObject columnEntry = columnValue.toObject();
        for (QString& columnName : columnEntry.keys()) {
            if (columnName == fieldName) {
                QJsonArray validationArray;
                for (auto rule : rules) {
                    QJsonObject ruleObj;
                    ruleObj["type"] = rule->getType();
                    if (rule->getType() == "minLength") {
                        ruleObj["value"] = rule->getValue().toInt();
                    }
                    validationArray.append(ruleObj);
                }
                columnEntry.insert("validation", validationArray);
                columnValue = columnEntry;
                break;
            }
        }
    }

    dbObj["clients"] = columnsArray;
    configFile["db"] = dbObj;
    saveConfigFile(configFile);
}
QJsonObject Config::getValidationRulesConfig() const {
    return configFile.value("validationRules").toObject();
}

