#pragma once

#include <QThread>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

class WorkerThread : public QThread {
    Q_OBJECT
public:
    WorkerThread();

    void run() override;
    void stop();

    boost::asio::io_context& getContext();
private:
    boost::asio::io_context mContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> mWorkGuard;
};

