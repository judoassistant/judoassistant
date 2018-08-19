#include <iostream>
#include "widgets/qutejudo_widget.hpp"
#include "applications/qutejudo_application.hpp"

#include "core.hpp"
// #include "stores/tournament_store.hpp"

int main(int argc, char *argv[]) {
    QutejudoApplication app(argc, argv);
    QutejudoWidget window;
    window.show();

    return app.exec();
}
