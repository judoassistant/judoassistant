#include "stores/player_store.hpp"

PlayerStore::PlayerStore(PlayerId id, const std::string & firstName, const std::string & lastName, std::optional<PlayerAge> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<PlayerWeight> weight, std::optional<PlayerCountry> country, std::optional<PlayerSex> sex)
    : mId(id)
    , mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
    , mRank(rank)
    , mClub(club)
    , mWeight(weight)
    , mCountry(country)
    , mSex(sex)
{}

const std::string & PlayerStore::getFirstName() const {
    return mFirstName;
}

const std::string & PlayerStore::getLastName() const {
    return mLastName;
}

const std::optional<PlayerAge> & PlayerStore::getAge() const {
    return mAge;
}

const PlayerId & PlayerStore::getId() const {
    return mId;
}

int PlayerRank::toInt() const {
    return static_cast<int>(mValue);
}

std::string PlayerRank::toString() const {
    // TODO: handle translation
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
    // TODO: handle translation
    switch (mValue) {
        case DENMARK: return "Denmark";
        case UNITED_KINGDOM: return "United Kingdom";
        case FRANCE: return "France";
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
    return mWeight;
}

const std::optional<PlayerRank> & PlayerStore::getRank() const {
    return mRank;
}

const std::optional<PlayerCountry> & PlayerStore::getCountry() const {
    return mCountry;
}

const std::string & PlayerStore::getClub() const {
    return mClub;
}

void PlayerStore::eraseMatch(CategoryId categoryId, MatchId matchId) {
    mMatches.erase(std::make_pair(categoryId, matchId));
}

void PlayerStore::addMatch(CategoryId categoryId, MatchId matchId) {
    mMatches.insert(std::make_pair(categoryId, matchId));
}

const std::unordered_set<std::pair<CategoryId,MatchId>> & PlayerStore::getMatches() const {
    return mMatches;
}

bool PlayerStore::containsMatch(CategoryId categoryId, MatchId matchId) const {
    return mMatches.find(std::make_pair(categoryId, matchId)) != mMatches.end();
}

void PlayerStore::eraseCategory(CategoryId id) {
    mCategories.erase(id);
}

void PlayerStore::addCategory(CategoryId id) {
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
    mFirstName = firstName;
}

void PlayerStore::setLastName(const std::string & lastName) {
    mLastName = lastName;
}

void PlayerStore::setAge(std::optional<PlayerAge> age) {
    mAge = age;
}

void PlayerStore::setWeight(std::optional<PlayerWeight> weight) {
    mWeight = weight;
}

void PlayerStore::setRank(std::optional<PlayerRank> rank) {
    mRank = rank;
}

void PlayerStore::setCountry(std::optional<PlayerCountry> country) {
    mCountry = country;
}

void PlayerStore::setClub(const std::string & club) {
    mClub = club;
}

std::ostream & operator<<(std::ostream &out, const PlayerCountry &country) {
    return out << country.toString();
}

std::ostream & operator<<(std::ostream &out, const PlayerRank &rank) {
    return out << rank.toString();
}

std::string PlayerSex::toString() const {
    // TODO: handle translation
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
    mSex = sex;
}

const std::optional<PlayerSex> PlayerStore::getSex() const {
    return mSex;
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

