#ifndef VALIDATIONRULEDIALOG_H
#define VALIDATIONRULEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QStringList>
#include <qcheckbox>
#include <qcombobox>
#include "validation/validationrule.h"

class ValidationRuleDialog : public QDialog {
    Q_OBJECT

public:
    explicit ValidationRuleDialog(const QStringList& availableFields,
                                  const QString& currentField = "",
                                  const QList<ValidationRule*>& rules = {},
                                  QWidget* parent = nullptr);

    QString getSelectedField() const;
    QList<ValidationRule*> getRules() const;

private:
    void setupUI();

    QStringList fields;
    QString selectedField;
    QMap<QString, QList<ValidationRule*>> ruleSets;

    QComboBox* fieldSelector;
    QCheckBox* notEmptyCheckbox;
    QCheckBox* emailCheckbox;
    QLineEdit* minLengthInput;
};

#endif // VALIDATIONRULEDIALOG_H
