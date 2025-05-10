#ifndef VALIDATIONRULE_H
#define VALIDATIONRULE_H

#include <QString>
#include <QVariant>

struct ValidationResult {
    bool isValid;
    QString errorMessage;

    operator bool() const { return isValid; }
};

class ValidationRule {
public:
    virtual ~ValidationRule() = default;
    virtual QString getType() const = 0;
    virtual QVariant getValue() const = 0;
    virtual bool setValue(const QVariant& value) = 0;
    virtual bool validate(const QString& value) const = 0;
    virtual QString getDescription() const = 0;
};

#endif // VALIDATIONRULE_H
