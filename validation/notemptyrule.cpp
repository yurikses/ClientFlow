#include "notemptyrule.h"

ValidationResult NotEmptyRule::validate(const QString& value) const {
    if (value.trimmed().isEmpty()) {
        return { false, "Поле не должно быть пустым" };
    }
    return { true, "" };
}

QString NotEmptyRule::getDescription() const {
    return "Не может быть пустым";
}
