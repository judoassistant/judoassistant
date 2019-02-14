#include "core/log.hpp"
#include "ui/store_managers/worker_thread.hpp"

WorkerThread::WorkerThread()
    : mWorkGuard(boost::asio::make_work_guard(mContext))
{}

void WorkerThread::run() {
    log_debug().msg("worker thread started");
    try {
        mContext.run();
    }
    catch (std::exception& e)
    {
        log_error().field("msg", e.what()).msg("Worker thread caught exception");
    }
    log_debug().msg("worker thread stopped");
}

void WorkerThread::stop() {
    mWorkGuard.reset();
}

boost::asio::io_context& WorkerThread::getContext() {
    return mContext;
}

