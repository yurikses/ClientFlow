#include "minlengthrule.h"

MinLengthRule::MinLengthRule(int length) : minLength(length) {}

ValidationResult MinLengthRule::validate(const QString& value) const {
    if (value.length() < minLength) {
        return { false, QString("Минимум %1 символов").arg(minLength) };
    }
    return { true, "" };
}

QString MinLengthRule::getDescription() const {
    return QString("Минимум %1 символов").arg(minLength);
}
