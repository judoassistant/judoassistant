#pragma once

#include <QMainWindow>
#include <QString>
#include <QtWidgets>

#include "core.hpp"
#include "store_handlers/qstore_handler.hpp"

class CategoriesWidget : public QWidget {
    Q_OBJECT
public:
    CategoriesWidget(QStoreHandler & storeHandler);
public slots:
    void showCategoryCreateDialog();
private:
    QStoreHandler &mStoreHandler;
};

