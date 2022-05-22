#include "ui/widgets/confirm_action_dialog.hpp"

bool ConfirmActionDialog::confirmAction() {
    auto reply = QMessageBox::question(nullptr, tr("Are you sure?"), tr("This action will modify matches that have already started. Are you sure?"), QMessageBox::Yes | QMessageBox::Cancel);

    return reply == QMessageBox::Yes;
}

