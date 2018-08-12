#include <iostream>
#include <QApplication>
#include "widgets/qutejudo_widget.hpp"
#include "applications/qutejudo_application.hpp"


int main(int argc, char *argv[]) {
    QutejudoApplication app(argc, argv);
    QutejudoWidget window;
    window.show();

    return app.exec();
}
