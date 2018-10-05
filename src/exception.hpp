#pragma once

#include <stdexcept>

class NotImplementedException : public std::logic_error {
public:
    NotImplementedException() : std::logic_error("Method not yet implemented") {}
};

class ActionExecutionException : public std::runtime_error {
public:
    ActionExecutionException(const std::string & msg) : std::runtime_error(msg) {}
};

