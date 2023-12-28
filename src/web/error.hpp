#pragma once

#include <boost/system/detail/error_code.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/beast/http/status.hpp>
#include <optional>
#include <string>

enum class ErrorCode {
	BadRequest   = int(boost::beast::http::status::bad_request),
	Conflict     = int(boost::beast::http::status::conflict),
	Unauthorized = int(boost::beast::http::status::unauthorized),
	Forbidden    = int(boost::beast::http::status::forbidden),
	NotFound     = int(boost::beast::http::status::not_found),

	Internal       = int(boost::beast::http::status::internal_server_error),
	NotImplemented = int(boost::beast::http::status::not_implemented),
	Unavailable    = int(boost::beast::http::status::service_unavailable),
};

struct Error {
    Error(ErrorCode code, const std::string &message);

    static Error wrapError(Error &other, const std::string &message);
    static std::optional<Error> wrapBoostSystemError(boost::system::error_code ec, const std::string &message);
    static std::optional<Error> wrapHTTPStatus(boost::beast::http::status status, const std::string &message);

    ErrorCode code;
    std::string message;
};
