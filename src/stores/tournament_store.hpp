#pragma once

#include <unordered_map>
#include <stack>

#include "core.hpp"
#include "serialize.hpp"
#include "stores/player_store.hpp"
#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "stores/tatami_store.hpp"
#include "position_manager.hpp"

class PlayerStore;
class CategoryStore;
class MatchStore;

class TournamentStore {
public:
    TournamentStore();
    virtual ~TournamentStore();

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("name", mName));
        ar(cereal::make_nvp("players", mPlayers));
        ar(cereal::make_nvp("categories", mCategories));
        ar(cereal::make_nvp("tatamis", mTatamis));
    }

    const std::string & getName() const;
    void setName(const std::string & name);

    const std::unordered_map<PlayerId, std::unique_ptr<PlayerStore>> & getPlayers() const;
    void addPlayer(std::unique_ptr<PlayerStore> ptr);
    PlayerStore & getPlayer(PlayerId id);
    const PlayerStore & getPlayer(PlayerId id) const;
    std::unique_ptr<PlayerStore> erasePlayer(PlayerId id);
    bool containsPlayer(PlayerId id) const;

    const std::unordered_map<CategoryId, std::unique_ptr<CategoryStore>> & getCategories() const;
    const CategoryStore & getCategory(CategoryId id) const;
    CategoryStore & getCategory(CategoryId id);
    void addCategory(std::unique_ptr<CategoryStore> ptr);
    std::unique_ptr<CategoryStore> eraseCategory(CategoryId id);
    bool containsCategory(CategoryId id) const;

    const TatamiList & getTatamis() const;
    TatamiList & getTatamis();

    PlayerId generateNextPlayerId();
    CategoryId generateNextCategoryId();
    MatchId generateNextMatchId();

    template <typename T>
    PositionId generateNextPositionId(const PositionManager<T> &manager) {
        while (true) {
            PositionId id = mPositionIdGenerator();
            if (!manager.containsId(id))
                return id;
        }
    };

    // signals used for frontends. Called by actions
    virtual void changeTournament() {}

    virtual void changePlayers(std::vector<PlayerId> id) {}
    virtual void beginAddPlayers(std::vector<PlayerId> id) {}
    virtual void endAddPlayers() {}
    virtual void beginErasePlayers(std::vector<PlayerId> id) {}
    virtual void endErasePlayers() {}
    virtual void beginResetPlayers() {}
    virtual void endResetPlayers() {}

    virtual void addPlayersToCategory(CategoryId category, std::vector<PlayerId>) {}
    virtual void erasePlayersFromCategory(CategoryId category, std::vector<PlayerId>) {}

    virtual void changeCategories(std::vector<CategoryId> id) {}
    virtual void beginAddCategories(std::vector<CategoryId> id) {}
    virtual void endAddCategories() {}
    virtual void beginEraseCategories(std::vector<CategoryId> id) {}
    virtual void endEraseCategories() {}
    virtual void beginResetCategories() {}
    virtual void endResetCategories() {}

    virtual void changeMatches(CategoryId categoryId, std::vector<MatchId> matchIds) {}
    virtual void beginResetMatches(CategoryId categoryId) {}
    virtual void endResetMatches(CategoryId categoryId) {}

    virtual void changeTatamis(std::vector<TatamiLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks) {}
    virtual void beginAddTatamis(std::vector<size_t> ids) {}
    virtual void endAddTatamis() {}
    virtual void beginEraseTatamis(std::vector<size_t> ids) {}
    virtual void endEraseTatamis() {}

private:
    std::string mName;

    std::unordered_map<PlayerId, std::unique_ptr<PlayerStore>> mPlayers;
    std::unordered_map<CategoryId, std::unique_ptr<CategoryStore>> mCategories;
    TatamiList mTatamis;

    PlayerId::Generator mPlayerIdGenerator;
    CategoryId::Generator mCategoryIdGenerator;
    MatchId::Generator mMatchIdGenerator;
    PositionId::Generator mPositionIdGenerator;
};

