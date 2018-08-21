#pragma once

#include <unordered_map>
#include <stack>

#include "core.hpp"
#include "actions/action.hpp"
#include "stores/player_store.hpp"
#include "stores/category_store.hpp"

class TournamentStore {
public:
    TournamentStore();

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar("name", mName);
        ar("players", mPlayers);
        ar("categories", mCategories);
        ar("nextPlayerId", mNextPlayerId);
        ar("nextCategoryId", mNextCategoryId);
        ar("nextMatchId", mNextMatchId);
        // actions
    }

    const std::string & getName() const;
    void setName(const std::string & name);

    const std::unordered_map<Id, std::unique_ptr<PlayerStore>> & getPlayers() const;
    const std::unordered_map<Id, std::unique_ptr<CategoryStore>> & getCategories() const;
    std::unique_ptr<CategoryStore> & getCategory(Id id);

    void addPlayer(std::unique_ptr<PlayerStore> ptr);
    void dispatchAction(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<Action> && action);

    Id generateNextPlayerId();
    Id generateNextCategoryId();
    Id generateNextMatchId();

    // signals useful for Qt
    virtual void tournamentChanged(Id id) {}
    virtual void matchAdded(Id id) {}
    virtual void matchChanged(Id id) {}
    virtual void matchDeleted(Id id) {}
    virtual void categoryAdded(Id id) {}
    virtual void categoryChanged(Id id) {}
    virtual void categoryDeleted(Id id) {}
    virtual void playerAdded(Id id) {}
    virtual void playerChanged(Id id) {}
    virtual void playerDeleted(Id id) {}
private:
    std::string mName;

    std::unordered_map<Id, std::unique_ptr<PlayerStore>> mPlayers;
    std::unordered_map<Id, std::unique_ptr<CategoryStore>> mCategories;
    std::stack<std::unique_ptr<Action>> mActions;

    Id mNextPlayerId;
    Id mNextCategoryId;
    Id mNextMatchId;
};

