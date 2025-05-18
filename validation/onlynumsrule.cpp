#include <QRegularExpression>
#include "OnlyNumsRule.h"
QString OnlyNumsRule::getType() const {
    return "onlyNums";
}

QVariant OnlyNumsRule::getValue() const {
    return {};
}

bool OnlyNumsRule::setValue(const QVariant&) {
    return false; // У этого правила нет параметров
}

bool OnlyNumsRule::validate(const QString& value) const {
    QString trimmedValue = value.trimmed();

    // Проверяем, что строка не пустая
    if (trimmedValue.isEmpty()) {
        return false;
    }

    // Регулярное выражение: только цифры и спецсимволы (без букв!)
    QRegularExpression regex("^[\\d\\W_]+$"); // \d — цифры, \W — неслово (спецсимволы), _ добавлено явно

    return regex.match(trimmedValue).hasMatch();
}

QString OnlyNumsRule::getDescription() const {
    return "Можно вводить только цифры и символы, без букв";
}
