#pragma once

#include <array>

// Header declaring different types used for communication

using WebToken = std::array<uint8_t, 32>;

enum class UserRegistrationResponse {
    SERVER_ERROR,
    EMAIL_EXISTS,
    INVALID_PASSWORD,
    SUCCESSFUL,
};

enum class WebTokenRequestResponse {
    SERVER_ERROR,
    INCORRECT_CREDENTIALS,
    SUCCESSFUL,
};

enum class WebTokenValidationResponse {
    SERVER_ERROR,
    EXPIRED_TOKEN,
    INVALID_TOKEN,
    SUCCESSFUL,
};

enum class WebNameCheckResponse {
    SERVER_ERROR,
    OCCUPIED_OTHER_USER,
    OCCUPIED_SAME_TOURNAMENT,
    OCCUPIED_OTHER_TOURNAMENT,
    FREE,
};

enum class WebNameRegistrationResponse {
    SERVER_ERROR,
    SUCCESSFUL,
    OCCUPIED_OTHER_USER,
};

