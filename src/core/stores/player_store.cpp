#include "core/stores/player_store.hpp"

PlayerStore::PlayerStore(PlayerId id, const PlayerFields &fields)
    : mId(id)
    , mFields(fields)
{}

const std::string & PlayerStore::getFirstName() const {
    return mFields.firstName;
}

const std::string & PlayerStore::getLastName() const {
    return mFields.lastName;
}

const std::optional<PlayerAge> & PlayerStore::getAge() const {
    return mFields.age;
}

const PlayerId & PlayerStore::getId() const {
    return mId;
}

const std::optional<PlayerWeight> & PlayerStore::getWeight() const {
    return mFields.weight;
}

const std::optional<PlayerRank> & PlayerStore::getRank() const {
    return mFields.rank;
}

const std::optional<PlayerCountry> & PlayerStore::getCountry() const {
    return mFields.country;
}

const std::string & PlayerStore::getClub() const {
    return mFields.club;
}

void PlayerStore::eraseMatch(const CombinedId &combinedId) {
    mMatches.erase(combinedId);
}

void PlayerStore::addMatch(const CombinedId &combinedId) {
    assert(mMatches.find(combinedId) == mMatches.end());
    mMatches.insert(combinedId);
}

const std::unordered_set<CombinedId> & PlayerStore::getMatches() const {
    return mMatches;
}

bool PlayerStore::containsMatch(const CombinedId &combinedId) const {
    return mMatches.find(combinedId) != mMatches.end();
}

void PlayerStore::eraseCategory(CategoryId id) {
    auto it = mCategories.find(id);
    assert(it != mCategories.end());
    mCategories.erase(it);
}

void PlayerStore::addCategory(CategoryId id) {
    assert(mCategories.find(id) == mCategories.end());
    mCategories.insert(id);
}

const std::unordered_set<CategoryId> & PlayerStore::getCategories() const {
    return mCategories;
}

bool PlayerStore::containsCategory(CategoryId id) const {
    return mCategories.find(id) != mCategories.end();
}

void PlayerStore::setFirstName(const std::string & firstName) {
    mFields.firstName = firstName;
}

void PlayerStore::setLastName(const std::string & lastName) {
    mFields.lastName = lastName;
}

void PlayerStore::setAge(std::optional<PlayerAge> age) {
    mFields.age = age;
}

void PlayerStore::setWeight(std::optional<PlayerWeight> weight) {
    mFields.weight = weight;
}

void PlayerStore::setRank(std::optional<PlayerRank> rank) {
    mFields.rank = rank;
}

void PlayerStore::setCountry(std::optional<PlayerCountry> country) {
    mFields.country = country;
}

void PlayerStore::setClub(const std::string & club) {
    mFields.club = club;
}

void PlayerStore::setSex(const std::optional<PlayerSex> sex) {
    mFields.sex = sex;
}

const std::optional<PlayerSex> PlayerStore::getSex() const {
    return mFields.sex;
}

void PlayerStore::setBlueJudogiHint(bool blueJudogiHint) {
    mFields.blueJudogiHint = blueJudogiHint;
}

const bool PlayerStore::getBlueJudogiHint() const {
    return mFields.blueJudogiHint;
}

