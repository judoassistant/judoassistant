#include "stores/player_store.hpp"

PlayerStore::PlayerStore(Id id, const std::string & firstName, const std::string & lastName, uint8_t age)
    : mId(id)
    , mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
{}

const std::string & PlayerStore::getFirstName() const {
    return mFirstName;
}

const std::string & PlayerStore::getLastName() const {
    return mLastName;
}

const uint8_t & PlayerStore::getAge() const {
    return mAge;
}

const Id & PlayerStore::getId() const {
    return mId;
}
