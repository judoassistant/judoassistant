#pragma once

#include <string>
#include <cstdint>
#include <unordered_set>

#include "core.hpp"
#include "serialize.hpp"

class PlayerStore {
public:
    PlayerStore() {}
    PlayerStore(Id id, const std::string & firstName, const std::string & lastName, uint8_t age);

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(cereal::make_nvp("id", mId));
        ar(cereal::make_nvp("firstName", mFirstName));
        ar(cereal::make_nvp("lastName", mLastName));
        ar(cereal::make_nvp("age", mAge));
        ar(cereal::make_nvp("categories", mCategories));
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

