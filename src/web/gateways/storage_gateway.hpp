#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <functional>

#include "core/log.hpp"
#include "web/config/config.hpp"
#include "web/web_tournament_store.hpp"

// StorageGateway gets and updates tournaments from disk storage.
class StorageGateway {
public:
    StorageGateway(boost::asio::io_context &context, Logger &logger, const Config &config);

    typedef std::function<void (boost::system::error_code, WebTournamentStore*, std::chrono::milliseconds)> GetTournamentCallback;

    // GetTournament returns an existing tournaments from storage.
    void asyncGetTournament(const std::string tournamentID, GetTournamentCallback callback);

    typedef std::function<void (boost::system::error_code)> UpsertTournamentCallback;
    // UpsertTournament upserts a tournament to storage.
    void asyncUpsertTournament(const std::string tournamentID, UpsertTournamentCallback callback);

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;
    const Config &mConfig;
};
