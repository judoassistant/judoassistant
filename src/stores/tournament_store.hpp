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
    }

    const std::string & getName() const;
    void setName(const std::string & name);

    const std::unordered_map<Id, std::unique_ptr<PlayerStore>> & getPlayers() const;
    const std::unordered_map<Id, std::unique_ptr<CategoryStore>> & getCategories() const;
    std::unique_ptr<CategoryStore> & getCategory(Id id);

    void addPlayer(std::unique_ptr<PlayerStore> ptr);
    void dispatchAction(std::unique_ptr<Action> action);

    Id generateNextPlayerId();
    Id generateNextCategoryId();
    Id generateNextMatchId();
private:
    std::string mName;

    std::unordered_map<Id, std::unique_ptr<PlayerStore>> mPlayers;
    std::unordered_map<Id, std::unique_ptr<CategoryStore>> mCategories;
    std::stack<std::unique_ptr<Action>> mActions;

    Id mNextPlayerId;
    Id mNextCategoryId;
    Id mNextMatchId;
};

