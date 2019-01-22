#pragma once

#include <thread>
#include <vector>
#include <boost/asio.hpp> // TODO: Do not include boost convenience headers

#include "core.hpp"
#include "web/web_server_worker.hpp"
#include "web/web_server_database_worker.hpp"
#include "web/web_participant.hpp"

class WebServer {
public:
    WebServer();

    void run();
    void quit();

private:
    void tcpAccept();

    boost::asio::io_context mContext;

    boost::asio::ip::tcp::endpoint mEndpoint;
    boost::asio::ip::tcp::acceptor mAcceptor;

    std::unordered_set<std::shared_ptr<WebParticipant>> mParticipants;
    std::unordered_map<std::string, size_t> mLoadedTournament;

    std::vector<std::thread> mThreads;
    std::unique_ptr<WebServerDatabaseWorker> mDatabaseWorker;
    std::vector<std::unique_ptr<WebServerWorker>> mWorkers;
};
