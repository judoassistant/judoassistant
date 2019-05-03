#include "core/id.hpp"
#include "core/actions/action.hpp"
#include "core/stores/match_store.hpp"


class SetMatchPlayerAction : public Action {
public:
    SetMatchPlayerAction() = default;
    SetMatchPlayerAction(CategoryId categoryId, MatchId matchId, MatchStore::PlayerIndex playerIndex, std::optional<PlayerId> playerId);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mMatchId, mPlayerIndex, mPlayerId);
    }

private:
    CategoryId mCategoryId;
    MatchId mMatchId;
    MatchStore::PlayerIndex mPlayerIndex;
    std::optional<PlayerId> mPlayerId;
    std::optional<PlayerId> mOldPlayerId;
};

CEREAL_REGISTER_TYPE(SetMatchPlayerAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, SetMatchPlayerAction)
