#pragma once

#include <thread>
#include <vector>
#include <boost/asio.hpp> // TODO: Do not include boost convenience headers

#include "core/core.hpp"
#include "web/config.hpp"
#include "web/database.hpp"
#include "web/loaded_tournament.hpp"
#include "web/tcp_participant.hpp"

class WebServer {
public:
    WebServer(const Config &config);

    void run();
    void quit();

    typedef std::function<void (std::shared_ptr<LoadedTournament>)> ObtainTournamentCallback;
    void obtainTournament(const std::string &webName, ObtainTournamentCallback callback);

private:
    void work();
    void tcpAccept();
    void leave(std::shared_ptr<TCPParticipant> participant);
    void assignWebName(std::shared_ptr<TCPParticipant> participant, std::string webName);

    Config mConfig;
    boost::asio::io_context mContext;
    boost::asio::io_context::strand mStrand;

    boost::asio::ip::tcp::endpoint mEndpoint;
    boost::asio::ip::tcp::acceptor mAcceptor;

    std::unordered_set<std::shared_ptr<TCPParticipant>> mParticipants;
    std::unordered_map<std::string, std::shared_ptr<LoadedTournament>> mLoadedTournaments;

    std::vector<std::thread> mThreads;
    std::unique_ptr<Database> mDatabase;

    friend TCPParticipant;
};

