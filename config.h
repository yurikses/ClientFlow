#ifndef CONFIG_H
#define CONFIG_H
#include <QObject>
#include <QJsonObject>
#include "validation/validationrule.h"

struct FieldConfig {
    QString name;
    QStringList oldNames;
    QString format;
    int size;
    QString defaultValue;
    QString tableDesc;

    FieldConfig() : size(0) {}
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
    QList<QString> getFieldNamesForTable();
    QList<QString> getFieldDescForTable();
    QJsonObject getBDConfig();
    static Config& instance();
    TableConfig getTableConfig(const QString &tableName);
    QList<ValidationRule*> getValidationRulesForField(const QString& fieldName) const;
    void saveValidationRules(const QString& fieldName, const QList<ValidationRule*>& rules);
};

#endif // CONFIG_H
