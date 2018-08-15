#pragma once

#include <string>
#include <cstdint>
#include <unordered_set>

#include "core.hpp"

class PlayerStore {
public:
    PlayerStore(Id id, const std::string & firstName, const std::string & lastName, uint8_t age);

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & mId;
        ar & mFirstName;
        ar & mLastName;
        ar & mAge;
        ar & mCategories;
    }

    const std::unordered_set<Id> & getCategories() const;
    const std::string & getFirstName() const;
    const std::string & getLastName() const;
    const uint8_t & getAge() const;
    const Id & getId() const;
private:
    Id mId;
    std::string mFirstName;
    std::string mLastName;
    uint8_t mAge;

    std::unordered_set<Id> mCategories;
};

