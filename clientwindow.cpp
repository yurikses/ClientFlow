#include "clientwindow.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>

ClientWindow::ClientWindow(Database *db, QWidget *parent)
    : QDialog(parent), database(db), clientId(-1)
{
    setupUI();
}

ClientWindow::ClientWindow(int clientId, Database *db, QWidget *parent)
    : QDialog(parent), database(db), clientId(clientId)
{
    setupUI();
    populateFields(clientId);
}

void ClientWindow::setupUI()
{
    // Создаем поля для ввода данных
    nameLineEdit = new QLineEdit(this);
    phoneLineEdit = new QLineEdit(this);
    birthDayDateEdit = new QDateEdit(this);
    ageLineEdit = new QLineEdit(this);

    // Настройка виджетов
    birthDayDateEdit->setCalendarPopup(true);
    birthDayDateEdit->setDisplayFormat("yyyy-MM-dd");

    // Создаем форму для размещения полей
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Имя:", nameLineEdit);
    formLayout->addRow("Возраст:", ageLineEdit);
    formLayout->addRow("Номер телефона:", phoneLineEdit);
    formLayout->addRow("День рождения:", birthDayDateEdit);

    // Создаем кнопки
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ClientWindow::saveClient);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ClientWindow::cancel);

    // Создаем вертикальный макет
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);

    // Устанавливаем макет для окна
    setLayout(layout);

    // Устанавливаем заголовок окна
    setWindowTitle(clientId == -1 ? "Создание клиента" : "Редактирование клиента");
}

void ClientWindow::populateFields(int clientId)
{
    QSqlQuery query;
    query.prepare("SELECT name, age, phone, birthDay FROM clients WHERE id = :id");
    query.bindValue(":id", clientId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось получить данные клиента: " + query.lastError().text());
        return;
    }

    if (query.next()) {
        nameLineEdit->setText(query.value("name").toString());
        ageLineEdit->setText(query.value("age").toString());
        phoneLineEdit->setText(query.value("phone").toString());
        birthDayDateEdit->setDate(query.value("birthDay").toDate());
    } else {
        QMessageBox::warning(this, "Предупреждение", "Клиент с ID " + QString::number(clientId) + " не найден.");
        cancel();
    }
}

void ClientWindow::saveClient()
{
    QString name = nameLineEdit->text();
    int age = ageLineEdit->text().toInt();
    QString phone = phoneLineEdit->text();
    QDate birthDay = birthDayDateEdit->date();
    database->openConnection("clients.db");

    if (clientId == -1) {
        // Создание нового клиента
        if (database->insertData(name, age, phone, birthDay)) {

            emit accepted(); // Отправляем сигнал accepted
            close();
        }
    } else {
        // Обновление существующего клиента
        if (database->updateData(clientId, name, age, phone, birthDay)) {

            emit accepted(); // Отправляем сигнал accepted
            close();
        }
    }
}

void ClientWindow::cancel()
{
    close();
}
