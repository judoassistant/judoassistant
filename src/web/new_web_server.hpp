#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <thread>

#include "web/config/config.hpp"
#include "web/gateways/meta_service_gateway.hpp"
#include "web/gateways/storage_gateway.hpp"
#include "web/handlers/tcp_handler.hpp"
#include "web/handlers/web_handler.hpp"

class NewWebServer {
public:
    NewWebServer(const Config &config);

    // run launches the web server worker threads and componenets
    void run();

    // quit gracefully shuts down the web server
    void async_close();
private:
    // work is called by each worker thread
    void work();

    const Config &mConfig;
    boost::asio::io_context mContext;
    boost::asio::io_context::strand mStrand;
    Logger mLogger;
    std::vector<std::thread> mThreads;
    TCPHandler mTCPHandler;
    WebHandler mWebHandler;
    MetaServiceGateway mMetaServiceGateway;
    StorageGateway mStorageGateway;
};
