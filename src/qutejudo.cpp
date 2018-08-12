#include <iostream>
#include <QApplication>

#include "widgets/qutejudo.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Qutejudo window;
    window.show();

    return app.exec();
}
