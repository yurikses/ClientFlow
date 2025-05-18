#include "validationruledialog.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QJsonObject>
ValidationRuleDialog::ValidationRuleDialog(const QString& field, const QJsonArray& currentRules, QWidget* parent)
    : QDialog(parent), selectedField(field) {

    setWindowTitle(QString("Настройка валидации для '%1'").arg(field));

    // Инициализация элементов
    notEmptyCheckbox = new QCheckBox("Не может быть пустым");
    emailCheckbox = new QCheckBox("Email");
    onlyLettersCheckbox = new QCheckBox("Только буквы");
    onlyNumsCheckbox = new QCheckBox("Только числа");
    minLengthInput = new QLineEdit();
    minLengthInput->setPlaceholderText("Минимальная длина");

    // Загрузка существующих правил
    for (const QJsonValue& val : currentRules) {
        QJsonObject rule = val.toObject();
        QString type = rule.value("type").toString();

        if (type == "notEmpty") {
            notEmptyCheckbox->setChecked(true);
        } else if (type == "email") {
            emailCheckbox->setChecked(true);
        } else if (type == "minLength") {
            minLengthInput->setText(QString::number(rule.value("value").toDouble()));
        } else if (type == "onlySymbols") {
            onlyLettersCheckbox->setChecked(true);
        } else if (type == "onlyNums") {
            onlyNumsCheckbox->setChecked(true);
        }
    }

    setupUI();
}

ValidationRuleDialog::~ValidationRuleDialog() {
    // Удалять не нужно, так как Qt автоматически удаляет виджеты
}

void ValidationRuleDialog::setupUI() {
    QFormLayout* form = new QFormLayout();
    form->addRow(new QLabel(QString("Поле: %1").arg(selectedField)));
    form->addRow(notEmptyCheckbox);
    form->addRow(onlyLettersCheckbox);
    form->addRow(onlyNumsCheckbox);
    form->addRow(emailCheckbox);
    form->addRow("Минимальная длина", minLengthInput);

    QHBoxLayout* buttons = new QHBoxLayout();
    QPushButton* okBtn = new QPushButton("OK");
    QPushButton* cancelBtn = new QPushButton("Отмена");
    buttons->addWidget(okBtn);
    buttons->addWidget(cancelBtn);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addLayout(buttons);
    setLayout(mainLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

QJsonArray ValidationRuleDialog::getRules() const {
    QJsonArray result;

    // Не пустое
    if (notEmptyCheckbox->isChecked()) {
        result.append(QJsonObject{{"type", "notEmpty"}});
    }

    // Email
    if (emailCheckbox->isChecked()) {
        result.append(QJsonObject{{"type", "email"}});
    }
    //onlySymbol
    if (onlyLettersCheckbox->isChecked() && !onlyNumsCheckbox->isChecked()) {
        result.append(QJsonObject{{"type", "onlySymbols"}});
    }
    //onlyNums
    if (onlyNumsCheckbox->isChecked()) {
        result.append(QJsonObject{{"type", "onlyNums"}});
    }
    // Минимальная длина
    bool ok;
    int minLength = minLengthInput->text().toInt(&ok);
    if (ok && minLength > 0) {
        result.append(QJsonObject{{"type", "minLength"}, {"value", minLength}});
    }

    return result;
}

QString ValidationRuleDialog::getSelectedField() const {
    return selectedField;
}
