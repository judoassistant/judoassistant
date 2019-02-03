#include "core/log.hpp"

// TODO: Implement some sort of scoped RAII logging

Log::Log(const std::string &prefix)
    : mPrefix(prefix)
{}

std::string blue(const std::string & a) {
    std::string res = "\033[34m";
    res += a;
    res += "\033[0m";
    return res;
}

Log & Log::msg(const std::string &msg) {
    std::cout << blue(mPrefix) << ": " << msg;

    for (auto p : mFields)
        std::cout << " " << blue(p.first) << "=" << p.second;

    mFields.clear();

    std::cout << std::endl;

    return *this;
}

Log & log_info() {
    static Log instance("INFO");
    return instance;
}

Log & log_debug() {
    static Log instance("DEBUG");
    return instance;
}

Log & log_error() {
    static Log instance("ERROR");
    return instance;
}

Log & log_fatal() {
    static Log instance("FATAL");
    return instance;
}

Log & log_warning() {
    static Log instance("WARNING");
    return instance;
}

// std::ostream & operator<<(std::ostream & o, const QString &str) {
//     return o << str.toStdString();
// }

