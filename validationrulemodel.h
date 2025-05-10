#ifndef VALIDATIONRULEMODEL_H
#define VALIDATIONRULEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QVariant>
#include "validation/validationrule.h"

class ValidationRuleModel : public QAbstractTableModel {
    Q_OBJECT
    QList<ValidationRule*> rules;
    QStringList headers = {"Тип", "Значение"};
public:
    explicit ValidationRuleModel(QObject* parent = nullptr);
    ~ValidationRuleModel();

    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void setRules(const QList<ValidationRule*>& rules);
    void addRule(ValidationRule* rule);
    void removeRule(int index);
    QList<ValidationRule*> getRules() const;
};

#endif // VALIDATIONRULEMODEL_H
