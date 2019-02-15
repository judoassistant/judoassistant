#pragma once

#include <string>

struct Config {
    unsigned int port;
    unsigned int webPort;
    unsigned int workers;
    std::string postgres;
};

