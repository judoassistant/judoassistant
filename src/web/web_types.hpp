#pragma once

#include <array>

// Header declaring different types used for communication

typedef std::array<uint8_t, 32> Token;

enum class RegistrationResponse {
    SERVER_ERROR,
    EMAIL_EXISTS,
    INVALID_PASSWORD,
    SUCCESSFUL,
};

enum class TokenRequestResponse {
    SERVER_ERROR,
    INCORRECT_CREDENTIALS,
    SUCCESSFUL,
};

enum class TokenValidationResponse {
    SERVER_ERROR,
    EXPIRED_TOKEN,
    INVALID_TOKEN,
    SUCCESSFUL,
};
