#include "minlengthrule.h"

MinLengthRule::MinLengthRule(int length) : minLength(length) {}

QString MinLengthRule::getType() const {
    return "minLength";
}

QVariant MinLengthRule::getValue() const {
    return minLength;
}

bool MinLengthRule::setValue(const QVariant& value) {
    bool ok;
    int newLength = value.toInt(&ok);
    if (ok && newLength > 0) {
        minLength = newLength;
        return true;
    }
    return false;
}

bool MinLengthRule::validate(const QString& value) const {
    if (value.length() < minLength) {
        return false;
    }
    return true;
}

QString MinLengthRule::getDescription() const {
    return QString("Минимум %1 символов").arg(minLength);
}
