#pragma once

#include <QWidget>
#include "core/core.hpp"

class WarningWidget : public QWidget {
    Q_OBJECT
public:
    WarningWidget(const QString &warning, QWidget *parent = nullptr);

private:
    const QString mWarning;
};
