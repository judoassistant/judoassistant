#pragma once

#include "core/core.hpp"
#include <QApplication>
#include <QPalette>

class ScoreApplication : public QApplication {
public:
    ScoreApplication(int &argc, char *argv[]);
    int exec();
private:
    QStringList mArgs;
};
