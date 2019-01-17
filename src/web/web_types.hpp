#pragma once

#include <array>

// Header declaring different types used for communication

typedef std::array<uint8_t, 32> Token;

enum class UserRegistrationResponse {
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

enum class WebNameValidationResponse {
    SERVER_ERROR,
    OCCUPIED_OTHER_USER,
    OCCUPIED_SAME_TOURNAMENT,
    OCCUPIED_OTHER_TOURNAMENT,
    FREE,
};

enum class WebNameCheckResponse {
    SERVER_ERROR,
    OCCUPIED_OTHER_USER,
    OCCUPIED_SAME_USER,
    FREE,
};

enum class WebNameRegistrationResponse {
    SERVER_ERROR,
    SUCCESSFUL,
    OCCUPIED_OTHER_USER,
};

