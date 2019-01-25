#pragma once

#include <QApplication>
#include <QPalette>

#include "core/core.hpp"

class HubApplication : public QApplication {
public:
    HubApplication(int &argc, char *argv[]);
    int exec();
private:
    QStringList mArgs;
};
