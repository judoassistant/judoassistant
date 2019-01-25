#pragma once

#include <QValidator>

class OptionalValidator : public QValidator {
    Q_OBJECT
    public:
        OptionalValidator(QValidator *child, QObject *parent);
        void fixup(QString &input) const override;
        QValidator::State validate(QString &input, int &pos) const override;
    private:
        QValidator *mChild;
};
