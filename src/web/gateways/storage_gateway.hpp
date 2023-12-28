#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <functional>
#include <optional>

#include "core/log.hpp"
#include "web/error.hpp"
#include "web/config/config.hpp"
#include "web/web_tournament_store.hpp"

// StorageGateway gets and updates tournaments from disk storage.
class StorageGateway {
public:
    StorageGateway(boost::asio::io_context &context, Logger &logger, const Config &config);

    typedef std::function<void (std::optional<Error>, WebTournamentStore*, std::chrono::milliseconds)> GetTournamentCallback;

    // asyncGetTournament returns an existing tournaments from storage.
    void asyncGetTournament(const std::string tournamentID, GetTournamentCallback callback);

    typedef std::function<void (std::optional<Error>)> UpsertTournamentCallback;
    // asyncUpsertTournament upserts a tournament to storage.
    void asyncUpsertTournament(const std::string tournamentID, WebTournamentStore &tournament, UpsertTournamentCallback callback);

private:
    boost::asio::io_context &mContext;
    boost::asio::io_context::strand mStrand;
    Logger &mLogger;
    const Config &mConfig;
};
