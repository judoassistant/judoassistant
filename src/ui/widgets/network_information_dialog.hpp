#pragma once

#include <optional>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

#include "core/core.hpp"

class NetworkInformationDialog : public QDialog {
    Q_OBJECT
public:
    NetworkInformationDialog(QWidget *parent = nullptr);

private:
    // QWidget *mHostContent;
    // QSpinBox *mPortContent;
    // QLineEdit *mPasswordContent;
    // QCheckBox *mShouldRetryContent;
    // QSpinBox *mRetryIntervalContent;
    // QPushButton *mConnectButton;
};
