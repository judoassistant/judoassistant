#include "core.hpp"
#include "applications/qutejudo_application.hpp"

// TODO: Rename Qutejudo to Judonizer

int main(int argc, char *argv[]) {
    QutejudoApplication app(argc, argv);
    return app.exec();
}
