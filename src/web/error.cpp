#include "error.hpp"
#include <optional>

Error::Error(ErrorCode code, const std::string &message)
    : code(code), message(message) {}

Error Error::wrapError(Error &other, const std::string &message) {
    return Error(other.code, message + ": " + other.message);
}

ErrorCode mapBoostSystemError(boost::system::error_code ec) {
    switch (ec.value()) {
        case boost::system::errc::permission_denied:
            return ErrorCode::Unauthorized;
        default:
            return ErrorCode::Unavailable;
    }
}

std::optional<Error> Error::wrapBoostSystemError(boost::system::error_code ec, const std::string &message) {
    if (!ec) {
        return std::nullopt;
    }

    return std::make_optional<Error>(mapBoostSystemError(ec), message);
}

ErrorCode mapHTTPStatus(unsigned int status) {
    switch (status) {
    case int(boost::beast::http::status::bad_request):
        return ErrorCode::BadRequest;
    case int(boost::beast::http::status::conflict):
        return ErrorCode::Conflict;
    case int(boost::beast::http::status::unauthorized):
        return ErrorCode::Unauthorized;
    case int(boost::beast::http::status::forbidden):
        return ErrorCode::Forbidden;
    case int(boost::beast::http::status::not_found):
        return ErrorCode::NotFound;
    case int(boost::beast::http::status::internal_server_error):
        return ErrorCode::Internal;
    case int(boost::beast::http::status::not_implemented):
        return ErrorCode::NotImplemented;
    case int(boost::beast::http::status::service_unavailable):
        return ErrorCode::Unavailable;
    default:
        return ErrorCode::Unavailable;
    }
}

std::optional<Error> Error::wrapHTTPStatus(unsigned int status, const std::string &message) {
    if (status == int(boost::beast::http::status::ok)) {
        return std::nullopt;
    }

    return std::make_optional<Error>(mapHTTPStatus(status), message);
}

std::ostream& operator<<(std::ostream &o, const Error &error) {
    return o << "Status " << int(error.code) << ": " << error.message;
}
