#include "actions/tatami_actions.hpp"
#include "stores/tournament_store.hpp"

SetTatamiLocationAction::SetTatamiLocationAction(TournamentStore & tournament, CategoryId categoryId, MatchType type, std::optional<TatamiLocation> location, size_t seqIndex)
    : mCategoryId(categoryId)
    , mType(type)
    , mLocation(location)
    , mSeqIndex(seqIndex)
{}

void SetTatamiLocationAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    auto & tatamis = tournament.getTatamis();
    if (mLocation && !tatamis.containsTatami(*mLocation)) return;
    CategoryStore &category = tournament.getCategory(mCategoryId);

    mOldLocation = category.getTatamiLocation(mType);
    if (mOldLocation)
        mOldSeqIndex = tatamis.getSeqIndex(*mOldLocation, mCategoryId, mType);
    else
        mOldSeqIndex = 0; // not used

    tatamis.moveBlock(tournament, mCategoryId, mType, mOldLocation, mLocation, mSeqIndex);
    category.setTatamiLocation(mType, mLocation);
}

void SetTatamiLocationAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsCategory(mCategoryId)) return;

    auto & tatamis = tournament.getTatamis();
    if (mLocation && !tatamis.containsTatami(*mLocation)) return;

    CategoryStore &category = tournament.getCategory(mCategoryId);

    tatamis.moveBlock(tournament, mCategoryId, mType, mLocation, mOldLocation, mOldSeqIndex);
    category.setTatamiLocation(mType, mOldLocation);
}

SetTatamiCount::SetTatamiCount(TournamentStore & tournament, size_t count)
    : mCount(count)
{}

void SetTatamiCount::redoImpl(TournamentStore & tournament) {
    auto & tatamis = tournament.getTatamis();

    mOldCount = tatamis.tatamiCount();

    if (tatamis.tatamiCount() < mCount) {
        std::vector<size_t> ids;
        for (size_t i = tatamis.tatamiCount(); i < mCount; ++i)
            ids.push_back(i);

        tournament.beginAddTatamis(std::move(ids));
        while (tatamis.tatamiCount() < mCount)
            tatamis.pushTatami();
        tournament.endAddTatamis();
    }

    if (tatamis.tatamiCount() > mCount) {
        std::vector<size_t> ids;
        for (size_t i = mCount; i < tatamis.tatamiCount(); ++i)
            ids.push_back(i);

        tournament.beginEraseTatamis(std::move(ids));
        while (tatamis.tatamiCount() > mCount)
            mOldContents.push(tatamis.popTatami());
        tournament.endEraseTatamis();
    }
}

void SetTatamiCount::undoImpl(TournamentStore & tournament) {
    auto & tatamis = tournament.getTatamis();

    if (tatamis.tatamiCount() > mOldCount) {
        std::vector<size_t> ids;
        for (size_t i = mOldCount; i < tatamis.tatamiCount(); ++i)
            ids.push_back(i);

        tournament.beginEraseTatamis(std::move(ids));
        while (tatamis.tatamiCount() > mOldCount)
            tatamis.popTatami();
        tournament.endEraseTatamis();
    }

    if (tatamis.tatamiCount() < mOldCount) {
        std::vector<size_t> ids;
        for (size_t i = tatamis.tatamiCount(); i < mOldCount; ++i)
            ids.push_back(i);

        tournament.beginAddTatamis(std::move(ids));
        while (tatamis.tatamiCount() < mOldCount) {
            tatamis.recoverTatami(mOldContents.top());
            mOldContents.pop();
        }
        tournament.endAddTatamis();
    }
}

