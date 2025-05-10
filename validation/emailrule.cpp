#include "emailrule.h"
#include <QRegularExpression>

QString EmailRule::getType() const {
    return "email";
}

QVariant EmailRule::getValue() const {
    return {};
}

bool EmailRule::setValue(const QVariant&) {
    return false; // У этого правила нет параметров
}

bool EmailRule::validate(const QString& value) const {
    QRegularExpression regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!regex.match(value).hasMatch()) {
        return false;
    }
    return  true;
}

QString EmailRule::getDescription() const {
    return "Должно быть в формате email";
}
