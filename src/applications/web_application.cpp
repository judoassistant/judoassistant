#include <signal.h>
#include <chrono>

#include "core.hpp"
#include "version.hpp"
#include "log.hpp"
#include "web/web_server.hpp"

WebServer server;

void handle_interrupt(int signal){
    log_info().msg("Caught interrupt. Quitting");
    server.quit();
}

int main(int argc, char *argv[]) {
    signal(SIGINT,handle_interrupt);
    server.run();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}

