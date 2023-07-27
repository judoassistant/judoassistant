#include "web/new_web_server.hpp"
#include "web/config/config.hpp"

NewWebServer::NewWebServer(const Config &config)
    : mConfig(config)
    , mContext(config.workers)
    , mStrand(mContext)
    , mLogger()
    , mMetaServiceGateway(mContext, mLogger)
    , mStorageGateway(mContext, mLogger)
    , mTournamentController(mContext, mLogger, mStorageGateway)
    , mTCPHandler(mContext, mLogger, mConfig, mTournamentController, mMetaServiceGateway)
    , mWebHandler(mContext, mLogger, mConfig, mTournamentController)
{}

void NewWebServer::run() {
    // Call async listener methods
    mWebHandler.asyncListen();
    mTCPHandler.asyncListen();

    // Start worker threads
    mLogger.info("Starting worker threads", LoggerField("workerCount", mConfig.workers));
    for (size_t i = 1; i < mConfig.workers; ++i) {
        mThreads.emplace_back(&NewWebServer::work, this);
    }
    work();

    // Wait for threads to finish
    mLogger.info("Shutting down");
    for (auto &thread : mThreads) {
        thread.join();
    }
}

void NewWebServer::async_close() {
    mWebHandler.asyncClose();
    mTCPHandler.asyncClose();
}

void NewWebServer::work() {
    mContext.run();
}
