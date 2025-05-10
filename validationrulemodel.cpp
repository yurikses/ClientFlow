#include "validationrulemodel.h"

ValidationRuleModel::ValidationRuleModel(QObject* parent)
    : QAbstractTableModel(parent) {}

ValidationRuleModel::~ValidationRuleModel() {
    qDeleteAll(rules);
}

int ValidationRuleModel::rowCount(const QModelIndex&) const {
    return rules.size();
}

int ValidationRuleModel::columnCount(const QModelIndex&) const {
    return headers.size();
}

QVariant ValidationRuleModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= rules.size())
        return {};

    auto rule = rules[index.row()];
    if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return rule->getType();
        if (index.column() == 1)
            return rule->getValue().toString();
    }
    return {};
}

QVariant ValidationRuleModel::headerData(int section, Qt::Orientation, int role) const {
    if (role == Qt::DisplayRole && section < headers.size())
        return headers[section];
    return {};
}

bool ValidationRuleModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.column() == 1 && role == Qt::EditRole) {
        rules[index.row()]->setValue(value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags ValidationRuleModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == 1)
        flags |= Qt::ItemIsEditable;
    return flags;
}

void ValidationRuleModel::addRule(ValidationRule* rule) {
    beginInsertRows({}, rules.size(), rules.size());
    rules.append(rule);
    endInsertRows();
}

void ValidationRuleModel::removeRule(int index) {
    beginRemoveRows({}, index, index);
    delete rules.takeAt(index);
    endRemoveRows();
}

QList<ValidationRule*> ValidationRuleModel::getRules() const {
    return rules;
}
