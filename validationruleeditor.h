#ifndef VALIDATIONRULEEDITOR_H
#define VALIDATIONRULEEDITOR_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include "validationrulemodel.h"

class ValidationRuleEditor : public QWidget
{
    Q_OBJECT
    ValidationRuleModel model;
    QTableView* tableView;
    QPushButton* addBtn;
    QPushButton* removeBtn;
public:
    explicit ValidationRuleEditor(QWidget *parent = nullptr);
    void setRules(const QList<ValidationRule*>& rules);
    QList<ValidationRule*> getRules() const;
private slots:
    void addRule();
    void removeSelectedRule();
};

#endif // VALIDATIONRULEEDITOR_H
