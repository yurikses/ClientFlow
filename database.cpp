#include "database.h"
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "database.h"
#include <QJsonObject>
#include <QJsonArray>

Database::Database(QObject *parent) : QObject(parent) {}

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
