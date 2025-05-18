#ifndef ONLYNUMSRULE_H
#define ONLYNUMSRULE_H

#include "validationrule.h"

class OnlyNumsRule : public ValidationRule
{
public:
    QString getType() const override;
    QVariant getValue() const override;
    bool setValue(const QVariant& value) override;
    bool validate(const QString& value) const override;
    QString getDescription() const override;
};

#endif // ONLYNUMSRULE_H
