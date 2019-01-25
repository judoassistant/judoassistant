#include "ui/validators/optional_validator.hpp"

OptionalValidator::OptionalValidator(QValidator *child, QObject *parent)
    : QValidator(parent)
    , mChild(child)
{
    mChild->setParent(this);
}

void OptionalValidator::fixup(QString &input) const {
    if (input.isEmpty()) return;

    mChild->fixup(input);
}

QValidator::State OptionalValidator::validate(QString &input, int &pos) const {
    if (input.isEmpty())
        return QValidator::State::Acceptable;
    return mChild->validate(input, pos);
}

