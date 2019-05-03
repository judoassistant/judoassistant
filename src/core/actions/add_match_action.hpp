#pragma once

#include "core/actions/action.hpp"
#include "core/id.hpp"
#include "core/stores/match_store.hpp"

class TournamentStore;


// Only used when drawing matches and beginResetCategories has already been called
class AddMatchAction : public Action {
public:
    AddMatchAction() = default;
    // AddMatchAction(const TournamentStore & tournament, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId);
    AddMatchAction(MatchId id, CategoryId categoryId, MatchType type, const std::string &title, bool bye, std::optional<PlayerId> whitePlayerId, std::optional<PlayerId> bluePlayerId);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
    MatchId getMatchId();

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId);
        ar(mCategoryId);
        ar(mType);
        ar(mTitle);
        ar(mBye);
        ar(mWhitePlayerId);
        ar(mBluePlayerId);
    }

private:
    MatchId mId;
    CategoryId mCategoryId;
    MatchType mType;
    std::string mTitle;
    bool mBye;
    std::optional<PlayerId> mWhitePlayerId;
    std::optional<PlayerId> mBluePlayerId;
};

CEREAL_REGISTER_TYPE(AddMatchAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddMatchAction)

