#include <signal.h>
#include <chrono>


#include "core/core.hpp"
#include "core/log.hpp"
#include "core/version.hpp"
#include "web/config/config.hpp"
#include "web/web_server.hpp"

std::unique_ptr<WebServer> server;

void handleInterrupt(int signal){
    server->quit();
}

int main(int argc, char *argv[]) {
    signal(SIGINT,handleInterrupt);

    try {
        auto[configuration, code] = loadConfig(argc, argv);
        if (code != 0) {
            return code;
        }

        auto server = std::make_unique<WebServer>(std::move(configuration));
        server->run();
    }
    catch(const std::exception& e) {
        log_error().field("what", e.what()).msg("Exception occured");
        return 1;
    }

    return 0;
}

