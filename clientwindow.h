#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDialog>
#include <QMap>
#include "database.h"
#include "config.h"

class ClientWindow : public QDialog {
    Q_OBJECT

public:
    explicit ClientWindow(const QString &tableName, Database *db, QWidget *parent = nullptr);
    explicit ClientWindow(int id, const QString &tableName, Database *db, QWidget *parent = nullptr);

private:
    void setupUI();
    void populateFields(int id);
    void saveClient();
    bool validateFields();
    QString tableName;
    int clientId = -1;
    Database *database;
    QMap<QString,QVector<ValidationRule*>> fieldValidators;
    QMap<QString, QWidget*> fieldWidgets; // для доступа к полям по имени
};

#endif // CLIENTWINDOW_H
