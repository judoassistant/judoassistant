#pragma once

#include <string>

namespace Config {
    // TODO: Move into a json file and read on runtime
    constexpr size_t WORKER_COUNT = 2; // Number of threads to handle tournaments
    constexpr char POSTGRES_CONFIG[] = "user=svendcs host=127.0.0.1 dbname=judoassistant";
}

