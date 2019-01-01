#pragma once

#include <unordered_map>

#include "core.hpp"
#include "id.hpp"
#include "serialize.hpp"
#include "tatami/tatami_list.hpp"

class PlayerStore;
class CategoryStore;
class MatchStore;

enum class MatchType;

class TournamentStore {
public:
    TournamentStore();
    TournamentStore(TournamentId id);
    TournamentStore(const TournamentStore &other);
    TournamentStore(TournamentStore &&other) = default;
    virtual ~TournamentStore();

    TournamentId getId() const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("name", mName));
        ar(cereal::make_nvp("id", mId));
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
    bool containsMatch(CategoryId categoryId, MatchId matchId) const;

    const TatamiList & getTatamis() const;
    TatamiList & getTatamis();

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

    virtual void changeTatamis(std::vector<BlockLocation> locations, std::vector<std::pair<CategoryId, MatchType>> blocks);
    virtual void beginAddTatamis(std::vector<TatamiLocation> locations);
    virtual void endAddTatamis();
    virtual void beginEraseTatamis(std::vector<TatamiLocation> locations);
    virtual void endEraseTatamis();

private:
    TournamentId mId;
    std::string mName;

    std::unordered_map<PlayerId, std::unique_ptr<PlayerStore>> mPlayers;
    std::unordered_map<CategoryId, std::unique_ptr<CategoryStore>> mCategories;
    TatamiList mTatamis;
};

