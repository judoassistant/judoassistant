#pragma once

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>

class QStoreHandler;

class EditPlayerWidget : public QWidget {
    Q_OBJECT
public:
    EditPlayerWidget(QStoreHandler & storeHandler, QWidget *parent);

public slots:
    void setPlayer(PlayerId id);
private:
    QStoreHandler & mStoreHandler;
    QLineEdit *mFirstNameContent;
    QLineEdit *mLastNameContent;
    QSpinBox *mAgeContent;
    std::optional<PlayerId> mId;
};
