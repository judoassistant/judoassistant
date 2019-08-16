#include "ui/widgets/warning_widget.hpp"

WarningWidget::WarningWidget(const QString &warning, QWidget *parent)
    : QWidget(parent)
    , mWarning(warning)
{

}
