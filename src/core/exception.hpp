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

class AddressInUseException : public std::runtime_error {
public:
    AddressInUseException(int port) : std::runtime_error("Address already in use"), mPort(port) {}

    int getPort() const { return mPort; }

private:
    int mPort;
};

