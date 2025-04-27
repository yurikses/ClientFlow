#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDebug>
#include "database.h"
#include "config.h"
#include "clientwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), dbManager(new Database(this)), config(new Config(this)), clientsTableWidget(ui->tableWidget)
{
    ui->setupUi(this);
    clientsTableWidget = ui->tableWidget;
    connect(ui->addClientButton, &QPushButton::clicked, this, &MainWindow::openCreateClientWindow);

    if (!dbManager->openConnection("clients.db")) {
        return;
    }

    dbManager->createTable(config->getBDConfig());
    connect(clientsTableWidget, &QTableWidget::cellDoubleClicked, this, &MainWindow::openEditClientWindow);
    loadDataFromDatabase(clientsTableWidget);


}

MainWindow::~MainWindow() {
    dbManager->closeConnection();
    delete ui;
}

void MainWindow::loadDataFromDatabase(QTableWidget *clientsTableWidget)
{
    // Получаем имена полей из конфигурационного файла для таблицы "clients"
    QStringList fieldNames = config->getFieldNamesForTable();

    // Получаем данные из базы данных
    QList<QMap<QString, QVariant>> dataList = dbManager->selectData(fieldNames);

    // Получаем заголовки столбцов из конфигурационного файла
    QList<QString> headersList = config->getBDlist();

    // Определяем количество строк и столбцов
    int rowCount = dataList.size();
    int columnCount = headersList.size();

    // Устанавливаем количество строк и столбцов в QTableWidget
    clientsTableWidget->setRowCount(rowCount);
    clientsTableWidget->setColumnCount(columnCount);

    // Устанавливаем заголовки столбцов
    clientsTableWidget->setHorizontalHeaderLabels(headersList);

    // Заполняем QTableWidget данными
    for (int row = 0; row < rowCount; ++row) {
        QMap<QString, QVariant> rowData = dataList[row];
        for (int col = 0; col < columnCount; ++col) {
            QString header = headersList[col];
            QString fieldName = config->getFieldNamesForTable()[col];
            QTableWidgetItem *item = new QTableWidgetItem(rowData[fieldName].toString());
            clientsTableWidget->setItem(row, col, item);
        }
    }

    QMessageBox::information(this, "Успех", "Данные успешно загружены.");
}
void MainWindow::openCreateClientWindow()
{
    ClientWindow *clientWindow = new ClientWindow(dbManager, this);
    connect(clientWindow, &ClientWindow::accepted, this, &MainWindow::handleClientWindowAccepted);
    clientWindow->show();
}

void MainWindow::openEditClientWindow(int row)
{
    // Проверьте, что элемент существует перед доступом к нему
    if (!clientsTableWidget->item(row, 0)) {
        qCritical() << "Error: Row or column index out of range.";
        return;
    }

    int clientId = clientsTableWidget->item(row, 0)->text().toInt();
    ClientWindow *clientWindow = new ClientWindow(clientId, dbManager, this);
    connect(clientWindow, &ClientWindow::accepted, this, &MainWindow::handleClientWindowAccepted);
    clientWindow->show();
}

void MainWindow::handleClientWindowAccepted()
{
    // Перезагружаем данные после создания или редактирования клиента
    loadDataFromDatabase(clientsTableWidget);
}
