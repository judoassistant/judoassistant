#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/errc.hpp>
#include <memory>

#include "web/controllers/tournament_controller_session.hpp"
#include "web/gateways/storage_gateway.hpp"
#include "web/web_tournament_store.hpp"

static constexpr size_t FILE_HEADER_SIZE = 17;

StorageGateway::StorageGateway(boost::asio::io_context &context, Logger &logger)
    : mContext(context)
    , mStrand(mContext)
    , mLogger(logger)
{}

// GetTournament returns an existing tournaments from storage.
void StorageGateway::asyncGetTournament(const std::string tournamentID, GetTournamentCallback callback) {
    boost::asio::post(mStrand, [this]() {
        // const auto ec = boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory);
        // WebTournamentStore *tournament = nullptr;
        // std::chrono::milliseconds clockDiff;
        // boost::asio::post(mContext, std::bind(callback, ec, tournament, clockDiff));
    });
}

void StorageGateway::asyncUpsertTournament(const std::string tournamentID, UpsertTournamentCallback callback) {
    boost::asio::post(mStrand, [this]() {
        // const auto ec = boost::system::errc::make_error_code(boost::system::errc::success);
        // boost::asio::post(mContext, std::bind(callback, ec));
    });
}
