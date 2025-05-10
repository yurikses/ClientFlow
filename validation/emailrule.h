#ifndef EMAILRULE_H
#define EMAILRULE_H

#include "validationrule.h"

class EmailRule : public ValidationRule {
public:
    QString getType() const override;
    QVariant getValue() const override;
    bool setValue(const QVariant& value) override;
    bool validate(const QString& value) const override;
    QString getDescription() const override;
};

#endif // EMAILRULE_H
