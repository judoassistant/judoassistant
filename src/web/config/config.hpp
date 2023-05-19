#pragma once

#include <boost/filesystem/path.hpp>
#include <string>

struct Config {
    unsigned int port;
    unsigned int webPort;
    unsigned int workers;
    std::string postgres;
    boost::filesystem::path dataDirectory;
};

std::pair<Config, int> loadConfig(int argc, char *argv[]);
