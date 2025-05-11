#include "validationruledialog.h"
#include "validation/emailrule.h"
#include "validation/minlengthrule.h"
#include "validation/notemptyrule.h"
#include <QFormLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <qlineedit.h>
#include <qpushbutton.h>

ValidationRuleDialog::ValidationRuleDialog(const QStringList& availableFields,
                                           const QString& currentField,
                                           const QList<ValidationRule*>& rules,
                                           QWidget* parent)
    : QDialog(parent), fields(availableFields) {

    setWindowTitle("Настройка валидации");

    fieldSelector = new QComboBox(this);
    fieldSelector->addItems(fields);
    if (!currentField.isEmpty()) {
        fieldSelector->setCurrentText(currentField);
    }

    notEmptyCheckbox = new QCheckBox("Не пустое");
    emailCheckbox = new QCheckBox("Email");
    minLengthInput = new QLineEdit();
    minLengthInput->setPlaceholderText("Минимальная длина");

    for (auto rule : rules) {
        if (rule->getType() == "notEmpty") {
            notEmptyCheckbox->setChecked(true);
        } else if (rule->getType() == "email") {
            emailCheckbox->setChecked(true);
        } else if (rule->getType() == "minLength") {
            minLengthInput->setText(rule->getValue().toString());
        }
    }

    setupUI();
}

void ValidationRuleDialog::setupUI() {
    QFormLayout* form = new QFormLayout();
    form->addRow("Поле:", fieldSelector);
    form->addRow("Не пустое", notEmptyCheckbox);
    form->addRow("Email", emailCheckbox);
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

QString ValidationRuleDialog::getSelectedField() const {
    return fieldSelector->currentText();
}

QList<ValidationRule*> ValidationRuleDialog::getRules() const {
    QList<ValidationRule*> result;

    if (notEmptyCheckbox->isChecked()) {
        result.append(new NotEmptyRule());
    }
    if (emailCheckbox->isChecked()) {
        result.append(new EmailRule());
    }
    bool ok;
    int minLength = minLengthInput->text().toInt(&ok);
    if (ok && minLength > 0) {
        result.append(new MinLengthRule(minLength));
    }

    return result;
}
