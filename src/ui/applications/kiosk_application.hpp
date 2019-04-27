#pragma once

#include "core/core.hpp"
#include <QApplication>
#include <QPalette>

class KioskApplication : public QApplication {
public:
    KioskApplication(int &argc, char *argv[]);
    int exec();
private:
    QStringList mArgs;
};
