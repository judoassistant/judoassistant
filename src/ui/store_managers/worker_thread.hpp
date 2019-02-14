#pragma once

#include <boost/asio.hpp> // TODO: Do not include boost convenience headers
#include <QThread>

class WorkerThread : public QThread {
public:
    WorkerThread();

    void run() override;
    void stop();

    boost::asio::io_context& getContext();
private:
    boost::asio::io_context mContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
};
