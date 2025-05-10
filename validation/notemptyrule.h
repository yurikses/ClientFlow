#ifndef NOTEMPTYRULE_H
#define NOTEMPTYRULE_H

#include "validationrule.h"

class NotEmptyRule : public ValidationRule {
public:
    QString getType() const override;
    QVariant getValue() const override;
    bool setValue(const QVariant& value) override;
    bool validate(const QString& value) const override;
    QString getDescription() const override;
};

#endif // NOTEMPTYRULE_H
