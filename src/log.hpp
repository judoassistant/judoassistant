#pragma once

#include <sstream>
#include <vector>
#include <iostream>
#include <optional>
#include <QString>

class Log {
public:
    Log(const std::string &prefix);

    template<typename T>
    Log & field(const std::string &key, const T &val) {
        std::stringstream ss;
        ss << val;
        mFields.push_back(std::make_pair(key, ss.str()));
        return *this;
    }

    Log & msg(const std::string &msg);

private:
    const std::string mPrefix;
    std::vector<std::pair<std::string,std::string>> mFields;
};

template<typename T>
std::ostream &operator<<(std::ostream & o, const std::vector<T> & vec) {
    o << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        o << vec[i];
        if (i != vec.size()-1)
            o << "; ";
    }

    return o << "]";
}

template <typename T>
std::ostream & operator<<(std::ostream & o, std::optional<T> opt) {
    if (opt)
        return o << *opt;
    return o << "nullopt";
}

std::ostream & operator<<(std::ostream & o, const QString &str);

Log & log_info();
Log & log_debug(); // TODO: make this a null sink in production builds and have support for file output
Log & log_error();
Log & log_fatal();

