#ifndef MINLENGTHTULE_H
#define MINLENGTHTULE_H

#include "validationrule.h"

class MinLengthRule : public ValidationRule {
    int minLength;
public:
    explicit MinLengthRule(int length);
    QString getType() const override;
    QVariant getValue() const override;
    bool setValue(const QVariant& value) override;
    bool validate(const QString& value) const override;
    QString getDescription() const override;
};

#endif // MINLENGTHTULE_H
