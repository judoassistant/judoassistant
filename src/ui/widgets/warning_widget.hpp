#pragma once

#include <QWidget>
#include "core/core.hpp"

class WarningWidget : public QWidget {
    Q_OBJECT
public:
    WarningWidget(const QString &warning, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const QString mWarning;
};
