#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include "database.h"
#include "config.h"
#include "clientwindow.h"

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

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Database *dbManager;
    QTableWidget *clientsTableWidget;
    Config *config;
};
#endif // MAINWINDOW_H
