#include "emailrule.h"
#include <QRegularExpression>

ValidationResult EmailRule::validate(const QString& value) const {
    QRegularExpression regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!regex.match(value).hasMatch()) {
        return { false, "Неверный формат email" };
    }
    return { true, "" };
}

QString EmailRule::getDescription() const {
    return "Должно быть в формате email";
}
