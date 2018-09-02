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
    const std::unordered_map<Id, std::unique_ptr<CategoryStore>> & getCategories() const;
    CategoryStore & getCategory(Id id);

    void addPlayer(std::unique_ptr<PlayerStore> ptr);
    PlayerStore & getPlayer(Id id);

    Id generateNextPlayerId();
    Id generateNextCategoryId();
    Id generateNextMatchId();

    // signals useful for Qt
    virtual void tournamentChanged() {}
    virtual void matchAdded(Id id) {}
    virtual void matchChanged(Id id) {}
    virtual void matchDeleted(Id id) {}
    virtual void categoryAdded(Id id) {}
    virtual void categoryChanged(Id id) {}
    virtual void categoryDeleted(Id id) {}
    virtual void playerAdded(Id id) {}
    virtual void playerChanged(Id id) {}
    virtual void playerDeleted(Id id) {}
private:
    std::string mName;

    std::unordered_map<Id, std::unique_ptr<PlayerStore>> mPlayers;
    std::unordered_map<Id, std::unique_ptr<CategoryStore>> mCategories;

    Id mNextPlayerId;
    Id mNextCategoryId;
    Id mNextMatchId;
};

