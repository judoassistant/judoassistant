#pragma once

#include "core.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>

class ConnectDialog : public QDialog {
    Q_OBJECT
public:
    ConnectDialog(const std::optional<QString> &host, std::optional<unsigned int> port, QWidget *parent = nullptr);

public slots:
    void acceptClick();
    void cancelClick();
private:
    QLineEdit *mHostContent;
    QSpinBox *mPortContent;
};
