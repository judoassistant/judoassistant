#pragma once

#include "actions/action.hpp"
#include "id.hpp"
#include "stores/match_store.hpp"
#include "stores/tatami_store.hpp"

class SetTatamiLocationAction : public Action {
public:
    SetTatamiLocationAction() = default;
    SetTatamiLocationAction(CategoryId categoryId, MatchType type, std::optional<TatamiLocation> location, size_t seqIndex);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId);
        ar(mType);
        ar(mLocation);
        ar(mSeqIndex);
    }

private:
    CategoryId mCategoryId;
    MatchType mType;
    std::optional<TatamiLocation> mLocation;
    size_t mSeqIndex;

    // undo members
    std::optional<TatamiLocation> mOldLocation;
    size_t mOldSeqIndex;
};

CEREAL_REGISTER_TYPE(SetTatamiLocationAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetTatamiLocationAction)

class SetTatamiCountAction : public Action {
public:
    SetTatamiCountAction() = default;
    SetTatamiCountAction(size_t count);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCount);
    }

private:
    size_t mCount;

    // undo members
    size_t mOldCount;
    std::stack<std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>>> mOldContents;
};

CEREAL_REGISTER_TYPE(SetTatamiCountAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetTatamiCountAction)

// Erase Tatami
// Add Tatami
