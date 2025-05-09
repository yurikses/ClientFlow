#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include "database.h"
#include "config.h"
#include "statistic.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:


    void loadDataFromDatabase(QTableWidget *clientTableWidget);
    void openCreateClientWindow();
    void openEditClientWindow(int row);
    void handleClientWindowAccepted();

    void on_action_3_triggered();
    void on_searchClientsButton_clicked();

    void on_delClientButton_clicked();
    void handleSettingsWindowAccepted();
    void loadDataFromList(QList<QMap<QString,QVariant>> data,QTableWidget *clientsTableWidget);

    void on_action_4_triggered();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Database *dbManager;
    QTableWidget *clientsTableWidget;
    Config &config;
    Statistic *statisticTab;
};
#endif // MAINWINDOW_H
