#include "database.h"
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include "database.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>

Database::Database(QObject *parent) : QObject(parent) , config(Config::instance()){}

Database::~Database() {
    closeConnection();
}

bool Database::openConnection(const QString &dbName) {
    db = QSqlDatabase::addDatabase("QSQLITE"); // Use SQLite driver
    db.setDatabaseName(dbName);

    if (!db.open()) {
        qCritical() << "Error: Could not open database.";
        qCritical() << db.lastError().text();
        return false;
    }
    qDebug() << "Database opened successfully.";
    return true;
}

void Database::closeConnection() {
    if (db.isOpen()) {
        db.close();
        qDebug() << "Database closed.";
    }
}

bool Database::createTable(const QJsonObject &dbConfig) {
    for (const QString &tableName : dbConfig.keys()) {
        QJsonArray columnsArray = dbConfig.value(tableName).toArray();
        QString createTableQuery = "CREATE TABLE IF NOT EXISTS " + tableName + " (";

        QStringList columns;
        for (const QJsonValue &columnEntry : columnsArray) {
            if (columnEntry.isObject()) {
                QJsonObject columnObj = columnEntry.toObject();
                for (const QString &columnName : columnObj.keys()) {
                    QJsonObject columnDetails = columnObj.value(columnName).toObject();
                    QString format = columnDetails.value("format").toString();
                    QString defaultValue = columnDetails.value("defaultValue").toString();
                    QString tableDesc = columnDetails.value("tableDesc").toString();

                    QString columnDef = columnName + " " + format;
                    if (!defaultValue.isEmpty() && defaultValue != "NULL") {
                        columnDef += " DEFAULT " + defaultValue;
                    }
                    if (columnName == "id") {
                        columnDef += " PRIMARY KEY AUTOINCREMENT";
                    }
                    columns << columnDef;
                }
            }
        }
        columns << "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP";
        createTableQuery += columns.join(", ") + ");";
        qDebug() << createTableQuery;
        QSqlQuery query;
        if (!query.exec(createTableQuery)) {
            qCritical() << "Error: Failed to create table" << tableName;
            qCritical() << query.lastError().text();
            return false;
        }
        qDebug() << "Table" << tableName << "created or already exists.";
    }
    return true;
}

bool Database::insertData(const QString &name, int age, const QString &phone, const QDate &birthDay) {
    QSqlQuery query;
    query.prepare("INSERT INTO clients (name, age, phone, birthDay) VALUES (:name, :age, :phone, :birthDay)");
    query.bindValue(":name", name);
    query.bindValue(":age", age);
    query.bindValue(":phone", phone);
    query.bindValue(":birthDay", birthDay);

    if (!query.exec()) {
        qCritical() << "Error: Failed to insert data.";
        qCritical() << query.lastError().text();
        return false;
    }
    qDebug() << "Data inserted successfully.";
    return true;
}

bool Database::updateData(int id, const QString &name, int age, const QString &phone, const QDate &birthDay) {
    QSqlQuery query;
    query.prepare("UPDATE clients SET name = :name, age = :age, phone = :phone, birthDay = :birthDay WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":age", age);
    query.bindValue(":phone", phone);
    query.bindValue(":birthDay", birthDay);

    if (!query.exec()) {
        qCritical() << "Error: Failed to update data.";
        qCritical() << query.lastError().text();
        return false;
    }
    qDebug() << "Data updated successfully.";
    return true;
}

QList<QMap<QString, QVariant>> Database::searchData(const QStringList &fieldNames, const QString &searchTerm) {
    QList<QMap<QString, QVariant>> dataList;

    QString selectQuery = "SELECT ";
    selectQuery += fieldNames.join(", ");
    selectQuery += " FROM clients";

    if (!searchTerm.isEmpty()) {
        selectQuery += " WHERE ";
        QStringList conditions;
        for (const QString &field : fieldNames) {
            conditions.append(field + " LIKE :search");
        }
        selectQuery += conditions.join(" OR ");
    }

    QSqlQuery query;
    query.prepare(selectQuery);
    if (!searchTerm.isEmpty()) {
        query.bindValue(":search", "%"+searchTerm+"%" );
    }
    query.exec();
    while (query.next()) {
        QMap<QString, QVariant> row;
        for (const QString &fieldName : fieldNames) {
            row[fieldName] = query.value(fieldName);
        }
        dataList.append(row);
    }

    qDebug() << "Data selected successfully.";
    return dataList;
}

QList<QMap<QString, QVariant>> Database::selectData(const QStringList &fieldNames) {
    QList<QMap<QString, QVariant>> dataList;

    // Формируем SQL-запрос для выбора данных
    QString selectQuery = "SELECT ";
    selectQuery += fieldNames.join(", ");
    selectQuery += " FROM clients";

    QSqlQuery query(selectQuery);

    while (query.next()) {
        QMap<QString, QVariant> row;
        for (const QString &fieldName : fieldNames) {
            row[fieldName] = query.value(fieldName);
        }
        dataList.append(row);
    }

    qDebug() << "Data selected successfully.";
    return dataList;
}

bool Database::deleteData(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM clients WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qCritical() << "Error: Failed to delete data.";
        qCritical() << query.lastError().text();
        return false;
    }
    qDebug() << "Data deleted successfully.";
    return true;
}

QSqlQuery Database::executeQuery(const QString &queryStr) {
    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qCritical() << "Error: Failed to execute query:" << queryStr;
        qCritical() << query.lastError().text();
    } else {
        qDebug() << "Query executed successfully:" << queryStr;
    }
    return query;
}

bool Database::syncTableStructure(const QJsonObject &dbConfig) {
    for (const QString &tableName : dbConfig.keys()) {
        QJsonArray columnsArray = dbConfig.value(tableName).toArray();

        QSqlQuery query;
        if (!query.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
            qCritical() << "Failed to get table info for" << tableName;
            return false;
        }

        QList<QPair<QString, QString>> currentColumns;
        while (query.next()) {
            QString name = query.value("name").toString().toLower(); // нормализуем регистр
            QString type = query.value("type").toString().toUpper();
            currentColumns.append({name, type});

        }

        QList<FieldConfig> expectedFields = extractFieldConfigs(columnsArray);

        bool needsRecreate = (currentColumns.size()-1 != expectedFields.size());

        if (!needsRecreate) {
            for (int i = 0; i < currentColumns.size()-1; ++i) {
                if (currentColumns[i].first != expectedFields[i].name.toLower() ||
                    currentColumns[i].second != expectedFields[i].format.toUpper()) {
                    needsRecreate = true;
                    break;
                }
            }
        }

        if (!needsRecreate) {
            qDebug() << "Таблица" << tableName << "уже соответствует конфигурации.";
            continue;
        }

        QString newTableName = tableName + "_new";

        if (query.exec(QString("DROP TABLE IF EXISTS %1;").arg(newTableName))) {
            qDebug() << "Старая временная таблица" << newTableName << "удалена.";
        }

        QStringList columnDefs;
        for (const auto &field : expectedFields) {
            QString def = field.name + " " + field.format;
            if (field.name == "id") {
                def += " PRIMARY KEY AUTOINCREMENT";
            }
            columnDefs << def;
        }
        columnDefs << "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP";
        QString createNewTable = QString("CREATE TABLE %1 (%2);")
                                     .arg(newTableName, columnDefs.join(", "));
        qDebug() << "SQL для создания новой таблицы:" << createNewTable;

        if (!query.exec(createNewTable)) {
            qCritical() << "Ошибка при создании новой таблицы" << newTableName;
            qCritical() << query.lastError().text();
            return false;
        }

        QStringList newFields;
        QStringList oldFields;

        for (const auto &field : expectedFields) {
            newFields << field.name;

            QString matchedOldName;
            bool found = false;

            // Ищем совпадение по имени или oldNames
            for (const auto &col : currentColumns) {
                if (col.first == field.name.toLower()) {
                    matchedOldName = col.first;
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (const QString &oldName : field.oldNames) {
                    for (const auto &col : currentColumns) {
                        if (col.first == oldName.toLower()) {
                            matchedOldName = col.first;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
            }

            if (found) {
                oldFields << matchedOldName;
            } else {
                oldFields << "NULL"; // поле не найдено — оставляем NULL
            }
        }
        newFields << "created_at";
        oldFields << "created_at";

        QString copyData = QString("INSERT INTO %1 (%2) SELECT %3 FROM %4;")
                               .arg(newTableName, newFields.join(", "), oldFields.join(", "), tableName);
        qDebug() << "SQL для копирования данных:" << copyData;

        if (!query.exec(copyData)) {
            qCritical() << "Ошибка при копировании данных в новую таблицу.";
            qCritical() << query.lastError().text();
            return false;
        }

        if (!query.exec(QString("DROP TABLE %1;").arg(tableName))) {
            qCritical() << "Ошибка при удалении старой таблицы" << tableName;
            return false;
        }

        if (!query.exec(QString("ALTER TABLE %1 RENAME TO %2;").arg(newTableName, tableName))) {
            qCritical() << "Ошибка при переименовании новой таблицы в" << tableName;
            return false;
        }

        qDebug() << "Структура таблицы" << tableName << "успешно обновлена.";
    }

    return true;
}

QList<FieldConfig> Database::extractFieldConfigs(const QJsonArray &columnsArray) {
    QList<FieldConfig> fields;

    for (const QJsonValue &columnEntry : columnsArray) {
        if (columnEntry.isObject()) {
            QJsonObject columnObj = columnEntry.toObject();
            for (const QString &columnName : columnObj.keys()) {
                QJsonObject details = columnObj.value(columnName).toObject();

                FieldConfig field;
                field.name = columnName;
                field.format = details.value("format").toString().toUpper();
                field.size = details.value("size").toInt();
                field.defaultValue = details.value("defaultValue").toString();
                field.tableDesc = details.value("tableDesc").toString();

                QJsonArray oldNamesArray = details.value("oldNames").toArray();
                for (const QJsonValue &val : oldNamesArray) {
                    field.oldNames.append(val.toString());
                }

                fields.append(field);
            }
        }
    }

    return fields;
}
QSqlDatabase Database::getDb() {
    return db;
}

bool Database::insertRecord(const QString &table, const QVariantMap &data) {
    QStringList keys = data.keys();
    QStringList placeholders;
    for (const QString &key : keys) placeholders << ":" + key;

    QString queryStr = QString("INSERT INTO %1 (%2) VALUES (%3)")
                           .arg(table, keys.join(", "), placeholders.join(", "));

    QSqlQuery query;
    query.prepare(queryStr);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    if (!query.exec()) {
        qCritical() << "Ошибка при вставке данных в таблицу" << table;
        qCritical() << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::updateRecord(const QString &table, int id, const QVariantMap &data) {
    QStringList assignments;
    for (const QString &key : data.keys()) {
        assignments << key + " = :" + key;
    }

    QString queryStr = QString("UPDATE %1 SET %2 WHERE id = :id")
                           .arg(table, assignments.join(", "));

    QSqlQuery query;
    query.prepare(queryStr);
    query.bindValue(":id", id);

    for (auto it = data.begin(); it != data.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    if (!query.exec()) {
        qCritical() << "Ошибка при обновлении данных в таблице" << table;
        qCritical() << query.lastError().text();
        return false;
    }

    return true;
}

QVariantMap Database::selectRecord(const QString &table, int id) {
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM %1 WHERE id = :id").arg(table));
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        QVariantMap result;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); ++i) {
            result[record.fieldName(i)] = query.value(i);
        }
        return result;
    }

    return {};
}

bool Database::importExternalDatabase(const QString &externalDbPath) {
    // 1. Открываем внешнюю БД
    QSqlDatabase externalDb = QSqlDatabase::addDatabase("QSQLITE", "import_connection");
    externalDb.setDatabaseName(externalDbPath);

    if (!externalDb.open()) {
        qCritical() << "Не удалось открыть внешнюю базу данных:" << externalDb.lastError().text();
        return false;
    }

    qDebug() << "Внешняя база данных открыта.";

    // 2. Проверяем, есть ли таблица clients
    if (!externalDb.tables().contains("clients")) {
        qCritical() << "Таблица 'clients' не найдена во внешней базе.";
        externalDb.close();
        return false;
    }

    // 3. Получаем структуру таблицы clients
    QSqlQuery query(externalDb);
    if (!query.exec("PRAGMA table_info(clients)")) {
        qCritical() << "Ошибка получения структуры таблицы clients";
        externalDb.close();
        return false;
    }

    QList<QPair<QString, QString>> columns; // имя - тип
    while (query.next()) {
        QString name = query.value("name").toString().toLower();
        QString type = query.value("type").toString().toUpper();

        if (name != "created_at") { // игнорируем created_at
            columns.append({name, type});
        }
    }

    // 4. Формируем новый config.json на основе структуры
    QJsonArray columnsArray;
    for (const auto &col : columns) {
        QJsonObject columnObj;
        columnObj["size"] = 50; // можно улучшить: определение по типу
        columnObj["format"] = col.second;
        columnObj["defaultValue"] = "";
        columnObj["tableDesc"] = col.first; // временное описание
        QJsonObject columnEntry;
        columnEntry[col.first] = columnObj;
        columnsArray.append(columnEntry);
    }

    QJsonObject dbConfig;
    dbConfig["clients"] = columnsArray;

    QJsonObject fullConfig;
    fullConfig["db"] = dbConfig;

    Config::instance().saveConfigFile(fullConfig); // сохраняем обновлённый конфиг
    qDebug() << "Конфигурация успешно обновлена из внешней базы данных.";

    // 5. Копируем внешнюю базу в локальную папку
    db.close();
    QDir appDir = QDir::current();
    QString localDbPath = appDir.filePath("clients.db");

    qDebug() << QFile::remove(localDbPath); // удаляем старую копию, если есть
    if (!QFile::copy(externalDbPath, localDbPath)) {
        qCritical() << "Не удалось скопировать базу данных в локальную папку.";
        externalDb.close();
        openConnection(localDbPath);
        return false;
    }

    qDebug() << "База данных успешно импортирована и скопирована.";

    externalDb.close();
    QSqlDatabase::removeDatabase("import_connection");

    // 6. Переподключаемся к локальной БД
    if (!openConnection(localDbPath)) {
        qCritical() << "Не удалось переподключиться к локальной копии базы данных.";
        return false;
    }

    // 7. Создаём таблицы (если нужно)
    dbConfig = config.getBDConfig();
    if (!syncTableStructure(dbConfig)) {
        qCritical() << "Не удалось синхронизировать структуру таблиц.";
        return false;
    }

    qDebug() << "Структура таблиц успешно синхронизирована.";

    return true;
}
