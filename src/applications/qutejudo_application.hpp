#pragma once

#include <QApplication>
#include <QPalette>

class QutejudoApplication : public QApplication {
public:
    QutejudoApplication(int &argc, char *argv[]);
private:
    QPalette m_palette;
};
