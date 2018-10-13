#pragma once

#include "core.hpp"
#include <QApplication>
#include <QPalette>

class QutejudoApplication : public QApplication {
public:
    QutejudoApplication(int &argc, char *argv[]);
    int exec();
private:
    QPalette mPalette;
    QStringList mArgs;
};
