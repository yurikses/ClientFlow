#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include "database.h"
#include "statistic.h"
#include "config.h"
#include "clientwindow.h"
#include "settingsdialog.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), dbManager(new Database(this)), config(Config::instance()), clientsTableWidget(ui->tableWidget)
{
    ui->setupUi(this);
    clientsTableWidget = ui->tableWidget;
    connect(ui->addClientButton, &QPushButton::clicked, this, &MainWindow::openCreateClientWindow);

    if (!dbManager->openConnection("clients.db")) {
        return;
    }

    dbManager->createTable(config.getBDConfig());
    connect(clientsTableWidget, &QTableWidget::cellDoubleClicked, this, &MainWindow::openEditClientWindow);
    loadDataFromDatabase(clientsTableWidget);
    statisticTab = new Statistic(this, dbManager);
    ui->tabWidget->addTab(statisticTab, "Статистика");

}

MainWindow::~MainWindow() {
    dbManager->closeConnection();
    delete ui;
}

/**
 * @brief Функция для вставки данных в таблицу {QTableWidget *clientsTableWidget}
 * @param clientsTableWidget виждет таблицы
 * @arg
 */
void MainWindow::loadDataFromDatabase(QTableWidget *clientsTableWidget)
{
    QJsonObject dbConfig = config.getBDConfig(); // получаем настройки для всех таблиц

    if (!dbManager->syncTableStructure(dbConfig)) {
        qDebug() << "Failed to update table structure";
    }

    // Получаем имена полей из конфигурационного файла для таблицы "clients"
    QStringList fieldNames = config.getFieldNamesForTable();

    // Получаем данные из базы данных
    QList<QMap<QString, QVariant>> dataList = dbManager->selectData(fieldNames);

    // Получаем заголовки столбцов из конфигурационного файла
    QList<QString> headersList = config.getBDlist();

    // Определяем количество строк и столбцов
    int rowCount = dataList.size();
    int columnCount = headersList.size();

    // Устанавливаем количество строк и столбцов в QTableWidget
    clientsTableWidget->setRowCount(rowCount);
    clientsTableWidget->setColumnCount(columnCount);

    // Устанавливаем заголовки столбцов
    clientsTableWidget->setHorizontalHeaderLabels(headersList);
    // Растягиваем все колонки
    for (int col = 0; col < columnCount; ++col) {
        clientsTableWidget->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }

    // Заполняем QTableWidget данными
    for (int row = 0; row < rowCount; ++row) {
        QMap<QString, QVariant> rowData = dataList[row];
        for (int col = 0; col < columnCount; ++col) {
            QString header = headersList[col];
            QString fieldName = config.getFieldNamesForTable()[col];
            QTableWidgetItem *item = new QTableWidgetItem(rowData[fieldName].toString());
            clientsTableWidget->setItem(row, col, item);
        }
    }

}

void MainWindow::loadDataFromList(QList<QMap<QString,QVariant>> data,QTableWidget *clientsTableWidget){
    int rowCount = data.size();
    int columnCount = clientsTableWidget->columnCount();
    clientsTableWidget->clearContents();
    clientsTableWidget->setRowCount(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        QMap<QString, QVariant> rowData = data[row];
        for (int col = 0; col < columnCount; ++col) {
            QString fieldName = config.getFieldNamesForTable()[col];
            QTableWidgetItem *item = new QTableWidgetItem(rowData[fieldName].toString());
            clientsTableWidget->setItem(row, col, item);
        }
    }
}

void MainWindow::openCreateClientWindow()
{
    ClientWindow *clientWindow = new ClientWindow("clients", dbManager, this);
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
    ClientWindow *clientWindow = new ClientWindow(clientId, "clients", dbManager, this);
    connect(clientWindow, &ClientWindow::accepted, this, &MainWindow::handleClientWindowAccepted);
    clientWindow->show();
}

void MainWindow::handleClientWindowAccepted()
{
    // Перезагружаем данные после создания или редактирования клиента
    loadDataFromDatabase(clientsTableWidget);
    QMessageBox::information(this, "Успех", "Данные успешно записаны.");
}

void MainWindow::handleSettingsWindowAccepted()
{
    config.importJsonFile();
    loadDataFromDatabase(clientsTableWidget);

}

void MainWindow::on_action_3_triggered()
{
    settingsdialog *settingsUI = new settingsdialog(this);
    settingsUI->show();
    connect(settingsUI, &settingsdialog::accepted, this, &MainWindow::handleSettingsWindowAccepted);
}


void MainWindow::on_delClientButton_clicked()
{

    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();

    if (!selectedItems.isEmpty()) {

        int row = selectedItems.first()->row();


        QTableWidgetItem* firstItem = ui->tableWidget->item(row, 0);

        if (firstItem) {
            QString value = firstItem->text();
            dbManager->deleteData(value.toInt());
            loadDataFromDatabase(clientsTableWidget);
            QMessageBox::information(this, "Успех", "Запись успешно удалена.");
            qDebug() << "Значение из первого столбца:" << value;
        } else {
            qDebug() << "Первая ячейка пустая!";
        }
    } else {
        qDebug() << "Нет выделенных строк!";
    }
}

void MainWindow::on_searchClientsButton_clicked()
{
    try {
        QStringList fieldNames = config.getFieldNamesForTable();
        QString searchTerm = ui->searchDatalineEdit->text();
        auto results = dbManager->searchData(fieldNames, searchTerm);
        loadDataFromList(results,clientsTableWidget);
    } catch (const QSqlError &e) {
        QMessageBox::critical(this, "Ошибка", "Ошибка поиска: " + e.text());
    }
}

void MainWindow::on_action_4_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите базу данных", "", "*.db *.sqlite");
    if (filePath.isEmpty()) return;

    if (dbManager->importExternalDatabase(filePath)) {
        loadDataFromDatabase(clientsTableWidget);
        QMessageBox::information(this, "Успех", "База данных успешно импортирована.");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось импортировать базу данных.");
    }
}


void MainWindow::on_action_5_triggered()
{
    dbManager->closeConnection();
    QString appDir = QCoreApplication::applicationDirPath();
    QString dirPath = appDir + "/old_db_" + QDate::currentDate().toString("dd-MM-yyyy");
    QDir dir;
    if (!dir.mkpath(dirPath)) {
        qDebug() << "Failed to create directory:" << dirPath;
        return;
    }
    QString sourceDbPath = "clients.db";
    QString destDbPath = dirPath + "/old_clients.db";

    // Ensure the source file exists
    if (!QFile::exists(sourceDbPath)) {
        qDebug() << "Source file does not exist:" << sourceDbPath;
        return;
    }

    // Remove existing file at destination (optional)
    if (QFile::exists(destDbPath) && !QFile::remove(destDbPath)) {
        qDebug() << "Failed to remove existing file:" << destDbPath;
        return;
    }

    // Perform the copy
    if (QFile::copy(sourceDbPath, destDbPath)) {
        qDebug() << "File copied successfully to:" << destDbPath;
    } else {
        qDebug() << "Failed to copy file.";
    }
    QString dbPath = appDir + "/clients.db";
    QString configPath = appDir + "/config.json";
    // Delete existing file
    if (QFile::exists(dbPath)) {
        if (!QFile::remove(dbPath)) {
            qWarning() << "Failed to delete existing database file";
            return;
        }
        qDebug() << "Old DB file deleted successfully";
    }

    if (QFile::exists(configPath)) {
        if (!QFile::remove(configPath)) {
            qWarning() << "Failed to delete existing database file";
            return;
        }
        qDebug() << "Old JSON file deleted successfully";
    }
    config.importJsonFile();
    dbManager->openConnection("clients.db");
    dbManager->createTable(config.getBDConfig());
    loadDataFromDatabase(clientsTableWidget);
    QMessageBox::information(this, "Готово", "Новая база данных создана!");

}

