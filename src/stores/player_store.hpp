#pragma once

#include <string>
#include <cstdint>
#include <unordered_set>

#include "core.hpp"
#include "serialize.hpp"
#include "id.hpp"

class PlayerCountry {
public:
    // TODO: Find list of countries
    static const size_t SIZE = 3;
    enum Enum {
        DENMARK,
        UNITED_KINGDOM,
        FRANCE,
    };

    PlayerCountry() {}
    PlayerCountry(Enum value) : mValue(value) {}
    PlayerCountry(int value) : mValue(static_cast<Enum>(value)) {}

    std::string toString() const;
    int toInt() const;
    static std::vector<PlayerCountry> values();

    bool operator==(const PlayerCountry &other) const {
        return mValue == other.mValue;
    }

    bool operator<(const PlayerCountry &other) const {
        return mValue < other.mValue;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(mValue);
    }
private:
    Enum mValue;
};

std::ostream & operator<<(std::ostream &out, const PlayerCountry &country);

class PlayerRank {
public:
    static const size_t SIZE = 16;
    enum Enum {
        KYU_6,
        KYU_5,
        KYU_4,
        KYU_3,
        KYU_2,
        KYU_1,
        DAN_1,
        DAN_2,
        DAN_3,
        DAN_4,
        DAN_5,
        DAN_6,
        DAN_7,
        DAN_8,
        DAN_9,
        DAN_10,
    };

    PlayerRank() {}
    PlayerRank(Enum value) : mValue(value) {}
    PlayerRank(int value) : mValue(static_cast<Enum>(value)) {}

    std::string toString() const;
    int toInt() const;
    static std::vector<PlayerRank> values();

    bool operator==(const PlayerRank &other) const {
        return mValue == other.mValue;
    }

    bool operator<(const PlayerRank &other) const {
        return mValue < other.mValue;
    }

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(mValue);
    }
private:
    Enum mValue;
};

std::ostream & operator<<(std::ostream &out, const PlayerRank &rank);

class PlayerStore {
public:
    PlayerStore() {}
    PlayerStore(PlayerId id, const std::string & firstName, const std::string & lastName, std::optional<uint8_t> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<float> weight, std::optional<PlayerCountry> country);

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(cereal::make_nvp("id", mId));
        ar(cereal::make_nvp("firstName", mFirstName));
        ar(cereal::make_nvp("lastName", mLastName));
        ar(cereal::make_nvp("age", mAge));
        ar(cereal::make_nvp("rank", mRank));
        ar(cereal::make_nvp("club", mClub));
        ar(cereal::make_nvp("weight", mWeight));
        ar(cereal::make_nvp("country", mCountry));
        ar(cereal::make_nvp("categories", mCategories));
    }

    const std::string & getFirstName() const;
    const std::string & getLastName() const;
    const std::optional<uint8_t> & getAge() const;
    const std::optional<float> & getWeight() const;
    const std::optional<PlayerRank> & getRank() const;
    const std::optional<PlayerCountry> & getCountry() const;
    const std::string & getClub() const;
    const PlayerId & getId() const;

    void setFirstName(const std::string & firstName);
    void setLastName(const std::string & lastName);
    void setAge(std::optional<uint8_t> age);
    void setWeight(std::optional<float> weight);
    void setRank(std::optional<PlayerRank> rank);
    void setCountry(std::optional<PlayerCountry> country);
    void setClub(const std::string & club);

    const std::unordered_set<CategoryId, CategoryId::Hasher> & getCategories() const;
    void addCategory(CategoryId id);
    void eraseCategory(CategoryId id);
    bool containsCategory(CategoryId id) const;

    const std::unordered_set<std::pair<CategoryId,MatchId>, CategoryIdMatchIdHasher> & getMatches() const;
    void addMatch(CategoryId categoryId, MatchId matchId);
    void eraseMatch(CategoryId categoryId, MatchId matchId);
    bool containsMatch(CategoryId categoryId, MatchId matchId) const;
private:
    PlayerId mId;
    std::string mFirstName;
    std::string mLastName;
    std::optional<uint8_t> mAge;
    std::optional<PlayerRank> mRank;
    std::string mClub;
    std::optional<float> mWeight;
    std::optional<PlayerCountry> mCountry;

    std::unordered_set<CategoryId, CategoryId::Hasher> mCategories;
    std::unordered_set<std::pair<CategoryId,MatchId>, CategoryIdMatchIdHasher> mMatches;
};

