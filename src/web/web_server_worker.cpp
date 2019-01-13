#include "log.hpp"
#include "web/web_server_worker.hpp"

WebServerWorker::WebServerWorker()
    : mContext()
    , mWorkGuard(boost::asio::make_work_guard(mContext))
{

}

void WebServerWorker::run() {
    mContext.run();
}

void WebServerWorker::quit() {
    mWorkGuard.reset();
}


