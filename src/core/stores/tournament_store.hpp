#pragma once

#include <unordered_map>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"
#include "core/stores/tatami/tatami_list.hpp"

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
    void setId(TournamentId id);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId, mName, mWebName, mPlayers, mCategories, mTatamis);
    }

    const std::string & getName() const;
    void setName(const std::string & name);

    const std::string & getWebName() const;
    void setWebName(const std::string & name);

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

    virtual void changePlayers(const std::vector<PlayerId> &playerIds) {}
    virtual void beginAddPlayers(const std::vector<PlayerId> &playerIds) {}
    virtual void endAddPlayers(const std::vector<PlayerId> &playerIds) {}
    virtual void beginErasePlayers(const std::vector<PlayerId> &playerIds) {}
    virtual void endErasePlayers(const std::vector<PlayerId> &playerIds) {}
    virtual void beginResetPlayers() {}
    virtual void endResetPlayers() {}

    virtual void addPlayersToCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds) {}
    virtual void erasePlayersFromCategory(CategoryId categoryId, const std::vector<PlayerId> &playerIds) {}

    virtual void changeCategories(const std::vector<CategoryId>& categoryIds) {}
    virtual void beginAddCategories(const std::vector<CategoryId>& categoryIds) {}
    virtual void endAddCategories(const std::vector<CategoryId>& categoryIds) {}
    virtual void beginEraseCategories(const std::vector<CategoryId>& categoryIds) {}
    virtual void endEraseCategories(const std::vector<CategoryId>& categoryIds) {}
    virtual void beginResetCategories() {}
    virtual void endResetCategories() {}

    virtual void changeMatches(CategoryId categoryId, const std::vector<MatchId> &matchIds) {}
    virtual void beginResetMatches(CategoryId categoryId) {}
    virtual void endResetMatches(CategoryId categoryId) {}

    virtual void changeTatamis(const std::vector<BlockLocation> &locations, const std::vector<std::pair<CategoryId, MatchType>> &blocks) {}
    virtual void beginAddTatamis(const std::vector<TatamiLocation> &locations) {}
    virtual void endAddTatamis(const std::vector<TatamiLocation> &locations) {}
    virtual void beginEraseTatamis(const std::vector<TatamiLocation> &locations) {}
    virtual void endEraseTatamis(const std::vector<TatamiLocation> &locations) {}

private:
    TournamentId mId;
    std::string mName;
    std::string mWebName;

    std::unordered_map<PlayerId, std::unique_ptr<PlayerStore>> mPlayers;
    std::unordered_map<CategoryId, std::unique_ptr<CategoryStore>> mCategories;
    TatamiList mTatamis;
};

