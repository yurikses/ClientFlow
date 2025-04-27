#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include "database.h"

class ClientWindow : public QDialog {
    Q_OBJECT

public:
    explicit ClientWindow(Database *db, QWidget *parent = nullptr);
    explicit ClientWindow(int clientId, Database *db, QWidget *parent = nullptr);

signals:
    void accepted(); // Сигнал для уведомления о завершении операции

private slots:
    void saveClient();
    void cancel();

private:
    void setupUI();
    void populateFields(int clientId);

    QLineEdit *nameLineEdit;
    QLineEdit *phoneLineEdit;
    QDateEdit *birthDayDateEdit;
    QLineEdit *ageLineEdit;
    QDialogButtonBox *buttonBox;
    Database *database;
    int clientId;
};

#endif // CLIENTWINDOW_H
