#include "web/web_server.hpp"
#include "web/config/config.hpp"
#include "web/models/tournament_meta.hpp"
#include "web/models/user_meta.hpp"

WebServer::WebServer(const Config &config)
    : mConfig(config)
    , mContext(config.workers)
    , mStrand(mContext)
    , mLogger()
    , mMetaServiceGateway(mContext, mLogger, mConfig)
    , mStorageGateway(mContext, mLogger, mConfig)
    , mTournamentController(mContext, mLogger, mConfig, mStorageGateway, mMetaServiceGateway)
    , mTCPHandler(mContext, mLogger, mConfig, mTournamentController, mMetaServiceGateway)
    , mWebHandler(mContext, mLogger, mConfig, mTournamentController)
{}

void WebServer::run() {
    // Call async listener methods
    mWebHandler.asyncListen();
    mTCPHandler.asyncListen();

    // Start worker threads
    mLogger.info("Starting worker threads", LoggerField("workerCount", mConfig.workers));
    for (size_t i = 1; i < mConfig.workers; ++i) {
        mThreads.emplace_back(&WebServer::work, this);
    }
    work();

    // Wait for threads to finish
    mLogger.info("Shutting down");
    for (auto &thread : mThreads) {
        thread.join();
    }
}

void WebServer::async_close() {
    mWebHandler.asyncClose();
    mTCPHandler.asyncClose();
    mTournamentController.asyncClose();
}

void WebServer::work() {
    mContext.run();
}
