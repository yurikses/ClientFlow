#ifndef CONFIG_H
#define CONFIG_H
#include <QObject>
#include <QJsonObject>
#include "validation/validationrule.h"

struct FieldConfig {
    QString name;
    QString tableDesc;
    QString format;
    int size = 0;
    QString defaultValue;
    QStringList oldNames;

    // Список правил валидации
    QList<QPair<QString, QVariant>> validationRules; // {"minLength", 12}, {"notEmpty", {}}
};

struct TableConfig {
    QList<QString> headersList;
    QList<QString> DescList;
    QList<FieldConfig> fieldConfigs;
};

class Config : public QObject {
    Q_OBJECT

private:
    explicit Config(QObject *parent = nullptr);

public:
    QJsonObject configFile;
    TableConfig config;
    void saveConfigFile(const QJsonObject &config);
    QJsonObject loadConfigFile();

    QList<QString> getBDlist();
    void importJsonFile();
    QString getFildDescByName(const QString& FieldName);
    QList<QString> getFieldNamesForTable();
    QList<QString> getFieldDescForTable();
    QJsonObject getBDConfig();
    static Config& instance();
    TableConfig getTableConfig(const QString &tableName);
   // QList<ValidationRule*> getValidationRules(const QString& fieldName) const;
    void saveValidationRules(const QString& fieldName, const QList<ValidationRule*>& rules);
    QJsonObject getValidationRulesForField(const QString& fieldName) const;
    void setValidationRulesForField(const QString& fieldName, const QList<ValidationRule*>& rules);
    QJsonObject getValidationRulesConfig() const; // Получает правила из JSON
    void setValidationRulesConfig(const QJsonObject& validation); // Устанавливает правила
    QJsonArray getFieldValidation(const QString& fieldName) const;
    void setFieldValidation(const QString& fieldName, const QJsonArray& validation);
    QJsonObject getValidationRules() const ;

};

#endif // CONFIG_H
