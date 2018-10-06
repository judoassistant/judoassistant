#include <iostream>
#include "widgets/qutejudo_window.hpp"
#include "applications/qutejudo_application.hpp"

#include "core.hpp"
// #include "stores/tournament_store.hpp"

// TODO: Rename Qutejudo to Judonizer

int main(int argc, char *argv[]) {
    QutejudoApplication app(argc, argv);
    QutejudoWindow window;
    window.show();

    return app.exec();
}
