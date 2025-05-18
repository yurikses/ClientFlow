#ifndef ONLYSYMBOLSRULE_H
#define ONLYSYMBOLSRULE_H

#include "validationrule.h"

class OnlySymbolsRule : public ValidationRule
{
public:
    QString getType() const override;
    QVariant getValue() const override;
    bool setValue(const QVariant& value) override;
    bool validate(const QString& value) const override;
    QString getDescription() const override;
};

#endif // ONLYSYMBOLSRULE_H
