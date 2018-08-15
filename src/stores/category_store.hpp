#pragma once

#include <string>
#include <unordered_set>

#include "core.hpp"

class CategoryStore {
public:
    CategoryStore();

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & mId;
        ar & mName;
        ar & mPlayers;
    }

    const std::unordered_set<Id> & getPlayers() const;
    const std::string & getName() const;
    const Id & getId() const;

private:
    Id mId;
    std::string mName;
    std::unordered_set<Id> mPlayers;
};

