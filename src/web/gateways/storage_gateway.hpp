#pragma once

#include <functional>
#include <boost/asio/io_context.hpp>

#include "core/log.hpp"

// StorageGateway gets and updates tournaments from disk storage.
class StorageGateway {
public:
    StorageGateway(boost::asio::io_context &context, Logger &logger);

    struct GetTournamentResponse {
    };
    typedef std::function<void (GetTournamentResponse)> GetTournamentCallback;

    // GetTournament returns an existing tournaments from storage.
    void GetTournament(const int id, GetTournamentCallback callback);

    struct UpsertTournamentResponse {
    };
    typedef std::function<void (UpsertTournamentResponse)> UpsertTournamentCallback;
    // UpsertTournament upserts a tournament to storage.
    void UpsertTournament(const int id, UpsertTournamentCallback callback);

private:
    boost::asio::io_context &mContext;
    Logger &mLogger;
};
