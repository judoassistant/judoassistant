#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>

#include "stores/player_store.hpp"
#include "stores/category_store.hpp"

class TournamentStore {
public:
    TournamentStore();

    template<typename Archive>
    void serialize(Archieve & ar, const unsigned int version) {
        ar & mName;
        ar & mPlayers;
        ar & mCategories;
    }

    const std::string & getName() const;
    void setName(const std::string &) const;

    const std::unordered_set<PlayerStore*> & getPlayers() const;
    const std::unordered_set<CategoryStore*> & getCategories() const;
private:
    std::string mName;

    std::unordered_map<PlayerStore::Id, PlayerStore*> mPlayers;
    std::unordered_map<CategoryStore::Id, CategoryStore*> mCategories;

    PlayerStore::Id mNextPlayerId;
    PlayerStore::Id mNextCategoryId;
};

