#include "validatorconfigloader.h"
#include "notemptyrule.h"
#include "emailrule.h"
#include "minlengthrule.h"
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>

QMap<QString, QList<ValidationRule*>> ValidatorConfigLoader::loadFromJson(const QJsonObject& config) {
    QMap<QString, QList<ValidationRule*>> result;
    QJsonObject validation = config.value("validation").toObject();

    for (auto it = validation.begin(); it != validation.end(); ++it) {
        QJsonArray rules = it.value().toArray();
        for (const QJsonValue& ruleVal : rules) {
            QJsonObject ruleObj = ruleVal.toObject();
            QString type = ruleObj.value("type").toString();

            ValidationRule* rule = nullptr;
            if (type == "notEmpty") {
                rule = new NotEmptyRule();
            } else if (type == "email") {
                rule = new EmailRule();
            } else if (type == "minLength") {
                int minLength = ruleObj.value("value").toInt(3);
                rule = new MinLengthRule(minLength);
            }

            if (rule) {
                result[it.key()].append(rule);
            }
        }
    }

    return result;
}
