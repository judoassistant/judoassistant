#pragma once

#include "core.hpp"
#include <QApplication>
#include <QPalette>

class HubApplication : public QApplication {
public:
    HubApplication(int &argc, char *argv[]);
    int exec();
private:
    QStringList mArgs;
};
