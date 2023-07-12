#include "web/new_web_server.hpp"
#include "web/config/config.hpp"

NewWebServer::NewWebServer(const Config &config)
    : mConfig(config)
    , mContext(config.workers)
    , mStrand(mContext)
    , mLogger()
    , mTCPParticipantHandler(mContext, mLogger)
    , mWebParticipantHandler(mContext, mLogger)
    , mMetaServiceGateway(mContext, mLogger)
    , mStorageGateway(mContext, mLogger)
{}

void NewWebServer::run() {
    mWebParticipantHandler.listen();
    mTCPParticipantHandler.listen();

    mLogger.info("Starting worker threads", LoggerField("workerCount", mConfig.workers));
    for (size_t i = 1; i < mConfig.workers; ++i) {
        mThreads.emplace_back(&NewWebServer::work, this);
    }
    work();

    mLogger.info("Shutting down");
    for (auto &thread : mThreads) {
        thread.join();
    }
}

void NewWebServer::quit() {
    // TODO: Implement
}

void NewWebServer::work() {
    // mContext.run();
}
