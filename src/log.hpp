#pragma once

#include <sstream>
#include <vector>
#include <set>
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
    for (auto it = vec.begin(); it != vec.end(); ) {
        o << *it;

        std::advance(it, 1);
        if (it != vec.end())
            o << "; ";
    }

    return o << "]";
}

template<typename T, typename Comp, typename Alloc>
std::ostream &operator<<(std::ostream & o, const std::set<T, Comp, Alloc> & set) {
    o << "{";
    for (auto it = set.begin(); it != set.end(); ) {
        o << *it;

        std::advance(it, 1);
        if (it != set.end())
            o << "; ";
    }

    return o << "}";
}

template <typename T>
std::ostream & operator<<(std::ostream & o, std::optional<T> opt) {
    if (opt)
        return o << *opt;
    return o << "nullopt";
}

template <typename A, typename B>
std::ostream & operator<<(std::ostream & o, std::pair<A,B> pair) {
    return o << "(" << pair.first << "; " << pair.second << ")";
}

std::ostream & operator<<(std::ostream & o, const QString &str);

Log & log_info();
Log & log_debug(); // TODO: make this a null sink in production builds and have support for file output
Log & log_error();
Log & log_fatal();

