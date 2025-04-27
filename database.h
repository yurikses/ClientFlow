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

class Database : public QObject {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool openConnection(const QString &dbName);
    void closeConnection();
    bool createTable(const QJsonObject &dbConfig);
    bool insertData(const QString &name, int age, const QString &phone, const QDate &birthDay);
    bool updateData(int id, const QString &name, int age, const QString &phone, const QDate &birthDay);
    QList<QMap<QString, QVariant>> selectData(const QStringList &fieldNames);
    bool deleteData(int id);
    QSqlQuery executeQuery(const QString &queryStr);

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
