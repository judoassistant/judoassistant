#pragma once

#include <QMessageBox>
#include <QDialog>

class ConfirmActionDialog : public QDialog {
public:
    static bool confirmAction();
};

