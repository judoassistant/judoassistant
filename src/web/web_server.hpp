#pragma once

#include <boost/asio/io_context_strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include <vector>

#include "core/core.hpp"
#include "web/config.hpp"
#include "web/database.hpp"
#include "web/loaded_tournament.hpp"
#include "web/web_participant.hpp"
#include "web/tcp_participant.hpp"

class WebServer {
public:
    WebServer(const Config &config);

    void run();
    void quit();

    // Acquires ownership of a tournament. This is meant for overwritting tournament data
    typedef std::function<void (std::shared_ptr<LoadedTournament>)> AcquireTournamentCallback;
    void acquireTournament(const std::string &webName, AcquireTournamentCallback callback);

    // Gets a tournament. If it already exists it will be loaded.
    typedef std::function<void (std::shared_ptr<LoadedTournament>)> GetTournamentCallback;
    void getTournament(const std::string &webName, GetTournamentCallback callback);

    typedef std::function<void (bool)> SaveTournamentCallback;
    void saveTournament(std::shared_ptr<LoadedTournament> tournament, SaveTournamentCallback);

    void leave(std::shared_ptr<TCPParticipant> participant);
    void leave(std::shared_ptr<WebParticipant> participant);

private:
    void work();
    void tcpAccept();
    void assignWebName(std::shared_ptr<TCPParticipant> participant, std::string webName);

    void webAccept();

    Config mConfig;
    boost::asio::io_context mContext;
    boost::asio::io_context::strand mStrand;

    boost::asio::ip::tcp::endpoint mTCPEndpoint;
    boost::asio::ip::tcp::acceptor mTCPAcceptor;

    boost::asio::ip::tcp::endpoint mWebEndpoint;
    boost::asio::ip::tcp::acceptor mWebAcceptor;

    std::unordered_set<std::shared_ptr<TCPParticipant>> mParticipants;
    std::unordered_set<std::shared_ptr<WebParticipant>> mWebParticipants;
    std::unordered_map<std::string, std::shared_ptr<LoadedTournament>> mLoadedTournaments;

    std::vector<std::thread> mThreads;
    std::unique_ptr<Database> mDatabase;
};

