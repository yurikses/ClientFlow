#ifndef VALIDATORCONFIGLOADER_H
#define VALIDATORCONFIGLOADER_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include "validationrule.h"

class ValidatorConfigLoader {
public:
    static QMap<QString, QList<ValidationRule*>> loadFromJson(const QJsonObject& config);
};

#endif // VALIDATORCONFIGLOADER_H
