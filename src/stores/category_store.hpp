#pragma once

#include <string>
#include <unordered_set>

#include "stores/player_store.hpp"

class CategoryStore {
public:
    typedef Id int64_t;

    CategoryStore();

    template<typename Archive>
    void serialize(Archieve & ar, const unsigned int version) {
        ar & mId;
        ar & mName;
        ar & mPlayers;
    }

    const std::unordered_set<PlayerStore::Id> & getPlayers() const;
    const std::string & getName() const;
    const Id & getId() const;

private:
    Id mId;
    std::string mName;
    std::unordered_set<PlayerStore::Id> mPlayers;
};

