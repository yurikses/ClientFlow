#include "validationruleeditor.h"
#include "validation/emailrule.h"
#include "validation/minlengthrule.h"
#include "validation/notemptyrule.h"
#include <QFormLayout>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QHeaderView>

ValidationRuleEditor::ValidationRuleEditor(QWidget* parent) : QWidget(parent), model(this) {
    tableView = new QTableView(this);
    tableView->setModel(&model);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setVisible(false);

    addBtn = new QPushButton("Добавить правило");
    removeBtn = new QPushButton("Удалить правило");

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tableView);
    mainLayout->addLayout(btnLayout);
    setLayout(mainLayout);

    connect(addBtn, &QPushButton::clicked, this, &ValidationRuleEditor::addRule);
    connect(removeBtn, &QPushButton::clicked, this, &ValidationRuleEditor::removeSelectedRule);
}

void ValidationRuleEditor::setRules(const QList<ValidationRule*>& rules) {
    model.setRules(rules);
}

QList<ValidationRule*> ValidationRuleEditor::getRules() const {
    return model.getRules();
}

void ValidationRuleEditor::addRule() {
    QDialog dialog(this);
    QFormLayout form(&dialog);
    QComboBox type;
    type.addItem("Не пустое", "notEmpty");
    type.addItem("Email", "email");
    type.addItem("Минимальная длина", "minLength");

    QLineEdit value;
    form.addRow("Тип", &type);
    form.addRow("Значение", &value);

    connect(&type, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [&](int idx) {
                value.setEnabled(type.itemData(idx).toString() == "minLength");
            });

    if (dialog.exec() == QDialog::Accepted) {
        QString selectedType = type.currentData().toString();
        ValidationRule* newRule = nullptr;

        if (selectedType == "notEmpty") {
            newRule = new NotEmptyRule();
        } else if (selectedType == "email") {
            newRule = new EmailRule();
        } else if (selectedType == "minLength") {
            int minLength = value.text().toInt();
            newRule = new MinLengthRule(minLength);
        }

        if (newRule) {
            model.addRule(newRule);
        }
    }
}

void ValidationRuleEditor::removeSelectedRule() {
    if (tableView->currentIndex().isValid()) {
        model.removeRule(tableView->currentIndex().row());
    }
}
