#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>

#include "core.hpp"
#include "stores/player_store.hpp"
#include "stores/category_store.hpp"

class TournamentStore {
public:
    TournamentStore();

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & mName;
        ar & mPlayers;
        ar & mCategories;
    }

    const std::string & getName() const;
    void setName(const std::string & name);

    const std::unordered_map<Id, std::unique_ptr<PlayerStore>> & getPlayers() const;
    const std::unordered_map<Id, std::unique_ptr<CategoryStore>> & getCategories() const;

    void addPlayer(std::unique_ptr<PlayerStore> ptr);

    Id generateNextPlayerId();
private:
    std::string mName;

    std::unordered_map<Id, std::unique_ptr<PlayerStore>> mPlayers;
    std::unordered_map<Id, std::unique_ptr<CategoryStore>> mCategories;

    Id mNextPlayerId;
    Id mNextCategoryId;
};

