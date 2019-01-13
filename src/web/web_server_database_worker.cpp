#include "log.hpp"
#include "web/web_server_database_worker.hpp"
#include "config/web_server.hpp"

WebServerDatabaseWorker::WebServerDatabaseWorker()
    : mContext()
    , mWorkGuard(boost::asio::make_work_guard(mContext))
{

}

void WebServerDatabaseWorker::run() {
    pqxx::connection connection(Config::POSTGRES_CONFIG);

    mContext.run();
}

void WebServerDatabaseWorker::quit() {
    mWorkGuard.reset();
}

