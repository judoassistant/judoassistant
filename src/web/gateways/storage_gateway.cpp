#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>

#include "web/gateways/storage_gateway.hpp"

StorageGateway::StorageGateway(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mLogger(logger)
{}
