#include "notemptyrule.h"

QString NotEmptyRule::getType() const {
    return "notEmpty";
}

QVariant NotEmptyRule::getValue() const {
    return {};
}

bool NotEmptyRule::setValue(const QVariant&) {
    return false; // У этого правила нет параметров
}

bool NotEmptyRule::validate(const QString& value) const {
    if (value.trimmed().isEmpty()) {
        return false;
    }
    return true;
}

QString NotEmptyRule::getDescription() const {
    return "Не может быть пустым";
}
