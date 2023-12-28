#include <exception>
#include <signal.h>
#include <chrono>
#include <memory>
#include <stdexcept>


#include "web/web_server.hpp"
#include "core/log.hpp"

std::unique_ptr<WebServer> server;
std::unique_ptr<Config> config;

void handleInterrupt(int signal){
    server->async_close();
}

int main(int argc, char *argv[]) {
    // Initialize server
    try {
        config = std::make_unique<Config>(argc, argv);
        server = std::make_unique<WebServer>(*config);
    }
    catch(const std::invalid_argument &e) {
        // Invalid config file. The constructors log these errors themselves.
        return 1;
    }
    catch(const std::exception &e) {
        Logger logger;
        logger.error("Unable to initialize server", LoggerField(e));
        return 1;
    }

    // Start server
    try {
        signal(SIGINT, handleInterrupt);
        server->run();
    }
    catch(const std::exception& e) {
        Logger logger;
        logger.error("Caught server exception", LoggerField(e));
        return 1;
    }

    return 0;
}

