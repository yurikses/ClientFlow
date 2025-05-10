#ifndef EMAILRULE_H
#define EMAILRULE_H

#include "validationrule.h"

class EmailRule : public ValidationRule {
public:
    ValidationResult validate(const QString& value) const override;
    QString getDescription() const override;
};

#endif // EMAILRULE_H
