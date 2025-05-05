#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QJsonObject>
#include "config.h"



class Database : public QObject {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();
    bool syncTableStructure(const QJsonObject &dbConfig);
    bool openConnection(const QString &dbName);
    void closeConnection();
    bool createTable(const QJsonObject &dbConfig);
    bool insertData(const QString &name, int age, const QString &phone, const QDate &birthDay);
    bool updateData(int id, const QString &name, int age, const QString &phone, const QDate &birthDay);
    QList<QMap<QString, QVariant>> selectData(const QStringList &fieldNames);
    QList<QMap<QString, QVariant>> searchData(const QStringList &fieldNames, const QString &searchTerm = QString());
    bool deleteData(int id);
    QSqlQuery executeQuery(const QString &queryStr);
    QList<FieldConfig> extractFieldConfigs(const QJsonArray &columnsArray);
    bool insertRecord(const QString &table, const QVariantMap &data);
    bool updateRecord(const QString &table, int id, const QVariantMap &data);
    QVariantMap selectRecord(const QString &table, int id);
    QSqlDatabase getDb(); // добавь в приватных методах
private:
    QSqlDatabase db;
};

#endif // DATABASE_H
