#include <signal.h>
#include <chrono>
#include <fstream>

#include <boost/program_options.hpp>

#include "core/core.hpp"
#include "core/log.hpp"
#include "core/version.hpp"
#include "web/config.hpp"
#include "web/web_server.hpp"

namespace po = boost::program_options;

std::unique_ptr<WebServer> server;

void handleInterrupt(int signal){ // TODO: Handle platform agnostically
    server->quit();
}

int main(int argc, char *argv[]) {
    signal(SIGINT,handleInterrupt);

    try {
        Config configuration;
        std::string configFile;

        std::string defaultConfigPath = std::string(SYSCONF_DIR) + "/config.cfg";
        // Declare a group of options that will be allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
            ("version,v", "print version string")
            ("help", "produce help message")
            ("config,c", po::value<std::string>(&configFile)->default_value(defaultConfigPath), "name of a file of a configuration.")
            ;

        // Declare a group of options that will be allowed both on command line
        // and in config file
        po::options_description config("Configuration");
        config.add_options()
            ("port", po::value<unsigned int>(&configuration.port)->default_value(9000), "tcp server port")
            ("web-port", po::value<unsigned int>(&configuration.webPort)->default_value(9001), "web socket server port")
            ("postgres", po::value<std::string>(&configuration.postgres)->default_value(""), "postgres connection info")
            ("workers", po::value<unsigned int>(&configuration.workers)->default_value(std::thread::hardware_concurrency()), "name of worker threads to launch")
            ("data-dir", po::value<boost::filesystem::path>(&configuration.dataDirectory)->default_value("tournaments"), "directory to store tournament data")
            ;

        po::options_description cmdOptions;
        cmdOptions.add(generic).add(config);

        po::options_description configFileOptions;
        configFileOptions.add(config);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        po::variables_map vm;
        store(po::command_line_parser(argc, argv).options(cmdOptions).run(), vm);
        notify(vm);

        std::ifstream ifs(configFile);
        if (!ifs) {
            log_error().field("file", configFile).msg("Can not open config file");
            return 1;
        }
        else {
            store(parse_config_file(ifs, configFileOptions), vm);
            notify(vm);
        }

        if (vm.count("help")) {
            std::cout << cmdOptions << std::endl;
            return 0;
        }

        if (vm.count("version")) {
            std::cout << "JudoAssistant Version " << ApplicationVersion::current().toString() << std::endl;
            return 0;
        }

        server = std::make_unique<WebServer>(std::move(configuration));
        server->run();
    }
    catch(const std::exception& e) {
        log_error().field("what", e.what()).msg("Exception occured");
        return 1;
    }

    return 0;
}

