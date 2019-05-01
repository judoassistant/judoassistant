#pragma once

#include "core/core.hpp"
#include "core/actions/action.hpp"
#include "core/stores/player_store.hpp"

class TournamentStore;

class AddPlayersAction : public Action {
public:
    AddPlayersAction() = default;
    AddPlayersAction(TournamentStore & tournament, const std::vector<PlayerFields> &fields);
    AddPlayersAction(const std::vector<PlayerId> &ids, const std::vector<PlayerFields> &fields);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mIds);
        ar(mFields);
    }

private:
    std::vector<PlayerId> mIds;
    std::vector<PlayerFields> mFields;
};

CEREAL_REGISTER_TYPE(AddPlayersAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddPlayersAction)
