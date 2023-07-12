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
{
}

void NewWebServer::run() {
    mWebParticipantHandler.run();
    mTCPParticipantHandler.run();
}

void NewWebServer::quit() {

}
