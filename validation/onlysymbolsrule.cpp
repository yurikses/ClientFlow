#include <QRegularExpression>
#include "OnlySymbolsRule.h"
QString OnlySymbolsRule::getType() const {
    return "onlySymbols";
}

QVariant OnlySymbolsRule::getValue() const {
    return {};
}

bool OnlySymbolsRule::setValue(const QVariant&) {
    return false; // У этого правила нет параметров
}

bool OnlySymbolsRule::validate(const QString& value) const {
    QString trimmedValue = value.trimmed();

    // Проверяем, что строка не пустая
    if (trimmedValue.isEmpty()) {
        return false;
    }

    // Регулярное выражение: только буквы (кириллица + латиница), точка и пробел
    QRegularExpression regex("^[A-Za-zА-Яа-яЁё\\s.]+$", QRegularExpression::CaseInsensitiveOption);

    return regex.match(trimmedValue).hasMatch();
}

QString OnlySymbolsRule::getDescription() const {
    return "Можно вводить только буквы, точку и пробел";
}
