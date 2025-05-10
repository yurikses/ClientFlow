#ifndef NOTEMPTYRULE_H
#define NOTEMPTYRULE_H

#include "validationrule.h"

class NotEmptyRule : public ValidationRule {
public:
    ValidationResult validate(const QString& value) const override;
    QString getDescription() const override;
};

#endif // NOTEMPTYRULE_H
