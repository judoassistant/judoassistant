#pragma once

#include <unordered_map>
#include <stack>

#include "core.hpp"
#include "serialize.hpp"
#include "stores/player_store.hpp"
#include "stores/category_store.hpp"

class TournamentStore {
public:
    TournamentStore();
    virtual ~TournamentStore();

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("name", mName));
        ar(cereal::make_nvp("players", mPlayers));
        ar(cereal::make_nvp("categories", mCategories));
        ar(cereal::make_nvp("nextPlayerId", mNextPlayerId));
        ar(cereal::make_nvp("nextCategoryId", mNextCategoryId));
        ar(cereal::make_nvp("nextMatchId", mNextMatchId));
    }

    const std::string & getName() const;
    void setName(const std::string & name);

    const std::unordered_map<Id, std::unique_ptr<PlayerStore>> & getPlayers() const;
    void addPlayer(std::unique_ptr<PlayerStore> ptr);
    PlayerStore & getPlayer(Id id);
    const PlayerStore & getPlayer(Id id) const;
    std::unique_ptr<PlayerStore> erasePlayer(Id id);
    bool containsPlayer(Id id) const;

    const std::unordered_map<Id, std::unique_ptr<CategoryStore>> & getCategories() const;
    const CategoryStore & getCategory(Id id) const;
    CategoryStore & getCategory(Id id);
    void addCategory(std::unique_ptr<CategoryStore> ptr);
    std::unique_ptr<CategoryStore> eraseCategory(Id id);
    bool containsCategory(Id id) const;

    Id generateNextPlayerId();
    Id generateNextCategoryId();
    Id generateNextMatchId();

    // signals used for frontends. Called by actions
    virtual void changeTournament() {}

    virtual void changePlayers(std::vector<Id> id) {}
    virtual void beginAddPlayers(std::vector<Id> id) {}
    virtual void endAddPlayers() {}
    virtual void beginErasePlayers(std::vector<Id> id) {}
    virtual void endErasePlayers() {}
    virtual void beginResetPlayers() {}
    virtual void endResetPlayers() {}

    virtual void addPlayersToCategory(Id category, std::vector<Id>) {}
    virtual void erasePlayersFromCategory(Id category, std::vector<Id>) {}

    virtual void changeCategories(std::vector<Id> id) {}
    virtual void beginAddCategories(std::vector<Id> id) {}
    virtual void endAddCategories() {}
    virtual void beginEraseCategories(std::vector<Id> id) {}
    virtual void endEraseCategories() {}
    virtual void beginResetCategories() {}
    virtual void endResetCategories() {}

    virtual void changeMatches(std::vector<Id> id) {}
    virtual void beginAddMatches(std::vector<Id> id) {}
    virtual void endAddMatches() {}
    virtual void beginEraseMatches(std::vector<Id> id) {}
    virtual void endEraseMatches() {}
    virtual void beginResetMatches() {}
    virtual void endResetMatches() {}
private:
    std::string mName;

    std::unordered_map<Id, std::unique_ptr<PlayerStore>> mPlayers;
    std::unordered_map<Id, std::unique_ptr<CategoryStore>> mCategories;

    Id mNextPlayerId;
    Id mNextCategoryId;
    Id mNextMatchId;
};

