#ifndef VALIDATIONRULEDIALOG_H
#define VALIDATIONRULEDIALOG_H

#include <QDialog>
#include <QJsonArray>
#include <QCheckBox>
#include <QLineEdit>

class ValidationRuleDialog : public QDialog {
    Q_OBJECT

public:
    explicit ValidationRuleDialog(const QString& field, const QJsonArray& currentRules, QWidget* parent = nullptr);
    ~ValidationRuleDialog();

    // Возвращает обновлённые правила в формате JSON
    QJsonArray getRules() const;
    QString getSelectedField() const;

private:
    void setupUI();

    QString selectedField; // Поле, для которого настраиваем правила

    QCheckBox* notEmptyCheckbox;
    QCheckBox* emailCheckbox;
    QCheckBox* onlyNumsCheckbox;
    QCheckBox* onlyLettersCheckbox;
    QLineEdit* minLengthInput;
};

#endif // VALIDATIONRULEDIALOG_H
