#pragma once

#include <string>

struct UserMeta {
    int id;
    std::string email;
    bool isAdmin;
};

struct UserCredentials {
    std::string email;
    std::string password;
};
