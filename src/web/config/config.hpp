#pragma once

#include <boost/filesystem/path.hpp>
#include <string>

struct Config {
    Config(int argc, char *argv[]);

    unsigned int tcpPort;
    unsigned int webPort;
    unsigned int workers;
    std::string postgres;
    boost::filesystem::path dataDirectory;
};