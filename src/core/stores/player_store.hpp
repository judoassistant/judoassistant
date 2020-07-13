#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_set>

#include "core/core.hpp"
#include "core/hash.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"
#include "core/stores/player_age.hpp"
#include "core/stores/player_country.hpp"
#include "core/stores/player_rank.hpp"
#include "core/stores/player_sex.hpp"
#include "core/stores/player_weight.hpp"

struct PlayerFields {
    std::string firstName;
    std::string lastName;
    std::optional<PlayerAge> age;
    std::optional<PlayerRank> rank;
    std::string club;
    std::optional<PlayerWeight> weight;
    std::optional<PlayerCountry> country;
    std::optional<PlayerSex> sex;

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(firstName, lastName, age, rank, club, weight, country, sex);
    }
};

class PlayerStore {
public:
    PlayerStore() {}
    PlayerStore(const PlayerStore &other) = default;
    PlayerStore(PlayerId id, const PlayerFields &fields);

    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar(mId, mFields, mLastFinishTime, mCategories, mMatches);
    }

    const std::string & getFirstName() const;
    const std::string & getLastName() const;
    const std::optional<PlayerAge> & getAge() const;
    const std::optional<PlayerWeight> & getWeight() const;
    const std::optional<PlayerRank> & getRank() const;
    const std::optional<PlayerCountry> & getCountry() const;
    const std::string & getClub() const;
    const PlayerId & getId() const;
    const std::optional<PlayerSex> getSex() const;

    void setFirstName(const std::string & firstName);
    void setLastName(const std::string & lastName);
    void setAge(std::optional<PlayerAge> age);
    void setWeight(std::optional<PlayerWeight> weight);
    void setRank(std::optional<PlayerRank> rank);
    void setCountry(std::optional<PlayerCountry> country);
    void setClub(const std::string & club);
    void setSex(const std::optional<PlayerSex> sex);

    const std::unordered_set<CategoryId> & getCategories() const;
    void addCategory(CategoryId id);
    void eraseCategory(CategoryId id);
    bool containsCategory(CategoryId id) const;

    const std::unordered_set<std::pair<CategoryId,MatchId>> & getMatches() const;
    void addMatch(CategoryId categoryId, MatchId matchId);
    void eraseMatch(CategoryId categoryId, MatchId matchId);
    bool containsMatch(CategoryId categoryId, MatchId matchId) const;

    std::optional<std::chrono::milliseconds> getLastFinishTime() const;
    void setLastFinishTime(std::optional<std::chrono::milliseconds> lastFinishTime);

private:
    PlayerId mId;
    PlayerFields mFields;
    std::optional<std::chrono::milliseconds> mLastFinishTime;

    std::unordered_set<CategoryId> mCategories;
    std::unordered_set<std::pair<CategoryId,MatchId>> mMatches;
};

