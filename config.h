#ifndef CONFIG_H
#define CONFIG_H
#include <QObject>
#include <QJsonObject>

class Config : public QObject {
    Q_OBJECT

private:
    QJsonObject configFile;
    void saveConfigFile(const QJsonObject &config);
    QJsonObject loadConfigFile();

public:
    explicit Config(QObject *parent = nullptr);
    QList<QString> getBDlist();
    QList<QString> getFieldNamesForTable();
    QJsonObject getBDConfig();
};

#endif // CONFIG_H
