#pragma once

#include "actions/action.hpp"
#include "id.hpp"
#include "stores/match_store.hpp"
#include "stores/tatami_store.hpp"

class SetTatamiLocationAction : public Action {
public:
    SetTatamiLocationAction(TournamentStore & tournament, CategoryId categoryId, MatchType type, std::optional<TatamiLocation> location, size_t seqIndex);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

private:
    CategoryId mCategoryId;
    MatchType mType;
    std::optional<TatamiLocation> mLocation;
    size_t mSeqIndex;

    // undo members
    std::optional<TatamiLocation> mOldLocation;
    size_t mOldSeqIndex;
};

class SetTatamiCount : public Action {
public:
    SetTatamiCount(TournamentStore & tournament, size_t count);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

private:
    size_t mCount;
    size_t mOldCount;
    std::stack<std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>>> mOldContents;
};

// Erase Tatami
// Add Tatami
