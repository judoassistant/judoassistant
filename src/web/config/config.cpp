#include <boost/program_options.hpp>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <iostream>

#include "core/core.hpp"
#include "core/version.hpp"
#include "web/config/config.hpp"

namespace po = boost::program_options;

Config::Config(int argc, char *argv[]) {
        std::string configFile;

        std::string defaultConfigPath = std::string(SYSCONF_DIR) + "/config.cfg";
        // Declare a group of options that will be allowed only on command line
        po::options_description generic("Command-line Options");
        generic.add_options()
            ("version,v", "print version string")
            ("help", "produce help message")
            ("config,c", po::value<std::string>(&configFile)->default_value(defaultConfigPath), "configuration file path")
            ;

        // Declare a group of options that will be allowed both on command line
        // and in config file
        po::options_description config("Configuration Options");
        config.add_options()
            ("tcp-port", po::value<unsigned int>(&this->port)->default_value(9000), "tcp server port")
            ("web-port", po::value<unsigned int>(&this->webPort)->default_value(9001), "web socket server port")
            ("workers", po::value<unsigned int>(&this->workers)->default_value(std::thread::hardware_concurrency()), "number of worker threads to launch")
            ("data-dir", po::value<boost::filesystem::path>(&this->dataDirectory)->default_value("tournaments"), "directory to store tournament data")
            ;

        po::options_description description;
        description.add(generic).add(config);

        po::options_description configFileOptions;
        configFileOptions.add(config);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        po::variables_map vm;
        store(po::command_line_parser(argc, argv).options(description).run(), vm);
        notify(vm);

        if (vm.count("help")) {
            std::cout << description;
            throw std::invalid_argument("help argument");
        }

        if (vm.count("version")) {
            std::cout << "JudoAssistant Version " << ApplicationVersion::current().toString() << std::endl;
            throw std::invalid_argument("version argument");
        }

        std::ifstream ifs(configFile);
        if (!ifs) {
            std::cerr << "Unable to open config file \"" << configFile << "\"" << std::endl;
            throw std::invalid_argument("unable to open config file");
        }

        store(parse_config_file(ifs, configFileOptions), vm);
        notify(vm);
}
