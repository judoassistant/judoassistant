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

int PlayerRank::toInt() const {
    return static_cast<int>(mValue);
}

std::string PlayerRank::toString() const {
    switch (mValue) {
        case KYU_6: return "6th kyu";
        case KYU_5: return "5th kyu";
        case KYU_4: return "4th kyu";
        case KYU_3: return "3th kyu";
        case KYU_2: return "2nd kyu";
        case KYU_1: return "1st kyu";
        case DAN_1: return "1st dan";
        case DAN_2: return "2nd dan";
        case DAN_3: return "3th dan";
        case DAN_4: return "4th dan";
        case DAN_5: return "5th dan";
        case DAN_6: return "6th dan";
        case DAN_7: return "7th dan";
        case DAN_8: return "8th dan";
        case DAN_9: return "9th dan";
        case DAN_10: return "10th dan";
        default: return "";
    }
}

std::vector<PlayerRank> PlayerRank::values() {
    std::vector<PlayerRank> res;
    for (size_t i = 0; i < SIZE; ++i) {
        res.push_back(PlayerRank(static_cast<Enum>(i)));
    }

    return res;
}

std::string PlayerCountry::toString() const {
    switch (mValue) {
        case DENMARK: return "Denmark";
        case GREAT_BRITAIN: return "Great Britain";
        case FRANCE: return "France";
        default: return "";
    }
}

std::string PlayerCountry::countryCode() const {
    switch (mValue) {
        case DENMARK: return "DEN";
        case GREAT_BRITAIN: return "GBR";
        case FRANCE: return "FRA";
        default: return "";
    }
}

std::vector<PlayerCountry> PlayerCountry::values() {
    std::vector<PlayerCountry> res;
    for (size_t i = 0; i < SIZE; ++i) {
        res.push_back(PlayerCountry(static_cast<Enum>(i)));
    }

    return res;
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

void PlayerStore::eraseMatch(CategoryId categoryId, MatchId matchId) {
    mMatches.erase(std::make_pair(categoryId, matchId));
}

void PlayerStore::addMatch(CategoryId categoryId, MatchId matchId) {
    auto p = std::make_pair(categoryId, matchId);
    assert(mMatches.find(p) == mMatches.end());
    mMatches.insert(p);
}

const std::unordered_set<std::pair<CategoryId,MatchId>> & PlayerStore::getMatches() const {
    return mMatches;
}

bool PlayerStore::containsMatch(CategoryId categoryId, MatchId matchId) const {
    return mMatches.find(std::make_pair(categoryId, matchId)) != mMatches.end();
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

int PlayerCountry::toInt() const {
    return static_cast<int>(mValue);
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

std::ostream & operator<<(std::ostream &out, const PlayerCountry &country) {
    return out << country.toString();
}

std::ostream & operator<<(std::ostream &out, const PlayerRank &rank) {
    return out << rank.toString();
}

std::string PlayerSex::toString() const {
    switch (mValue) {
        case MALE: return "Male";
        case FEMALE: return "Female";
        default: return "";
    }
}

std::vector<PlayerSex> PlayerSex::values() {
    std::vector<PlayerSex> res;
    for (size_t i = 0; i < SIZE; ++i) {
        res.push_back(PlayerSex(static_cast<Enum>(i)));
    }

    return res;
}

int PlayerSex::toInt() const {
    return static_cast<int>(mValue);
}

void PlayerStore::setSex(const std::optional<PlayerSex> sex) {
    mFields.sex = sex;
}

const std::optional<PlayerSex> PlayerStore::getSex() const {
    return mFields.sex;
}

PlayerCountry::PlayerCountry(const std::string &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto country = PlayerCountry(i);
        if (country.toString() == str) {
            mValue = static_cast<Enum>(i);
            return;
        }
    }

    throw std::invalid_argument(str);
}

PlayerRank::PlayerRank(const std::string &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto rank = PlayerRank(i);
        if (rank.toString() == str) {
            mValue = static_cast<Enum>(i);
            return;
        }
    }

    throw std::invalid_argument(str);
}

PlayerSex::PlayerSex(const std::string &str) {
    for (int i = 0; i < static_cast<int>(SIZE); ++i) {
        auto sex = PlayerSex(i);
        if (sex.toString() == str) {
            mValue = static_cast<Enum>(i);
            return;
        }
    }

    throw std::invalid_argument(str);
}

PlayerWeight::PlayerWeight(float value)
    : mValue(value)
{
    if (mValue < min() || mValue > max())
        throw std::invalid_argument(std::to_string(value));
}

PlayerWeight::PlayerWeight(const std::string &str) {
    try {
        if (!str.empty() && str.front() == '-')
            mValue = std::stof(str.substr(1, str.size()-1));
        mValue = std::stof(str);
    }
    catch (const std::exception &e) {
        throw std::invalid_argument(str);
    }

    if (mValue < min() || mValue > max())
        throw std::invalid_argument(str);
}

std::string PlayerWeight::toString() const {
    return std::to_string(mValue);
}

float PlayerWeight::toFloat() const {
    return mValue;
}

float PlayerWeight::max() {
    return 400;
}

float PlayerWeight::min() {
    return 0;
}

std::ostream & operator<<(std::ostream &out, const PlayerWeight &weight) {
    return out << weight.toString();
}

PlayerAge::PlayerAge(int value)
    : mValue(value)
{
    if (mValue < min() || mValue > max())
        throw std::invalid_argument(std::to_string(value));
}

PlayerAge::PlayerAge(const std::string &str) {
    try {
        mValue = std::stoi(str);
    }
    catch (const std::exception &e) {
        throw std::invalid_argument(str);
    }

    if (mValue < min() || mValue > max())
        throw std::invalid_argument(str);
}

std::string PlayerAge::toString() const {
    return std::to_string(mValue);
}

int PlayerAge::toInt() const {
    return mValue;
}

int PlayerAge::max() {
    return 255;
}

int PlayerAge::min() {
    return 0;
}

std::ostream & operator<<(std::ostream &out, const PlayerAge &age) {
    return out << age.toString();
}

PlayerRank::PlayerRank(int value)
    : mValue(static_cast<Enum>(value))
{
    if (mValue < 0 || mValue >= SIZE)
        throw std::invalid_argument(std::to_string(value));
}

PlayerCountry::PlayerCountry(int value)
    : mValue(static_cast<Enum>(value))
{
    if (mValue < 0 || mValue >= SIZE)
        throw std::invalid_argument(std::to_string(value));
}

PlayerSex::PlayerSex(int value)
    : mValue(static_cast<Enum>(value))
{
    if (mValue < 0 || mValue > 1)
        throw std::invalid_argument(std::to_string(value));
}

