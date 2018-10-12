#pragma once

#include <string>
#include <unordered_set>
#include <map>

#include "core.hpp"
#include "stores/match_store.hpp"
#include "draw_systems/draw_system.hpp"
#include "rulesets/ruleset.hpp"
#include "id.hpp"

class CategoryStore {
public:
    CategoryStore() {}
    CategoryStore(CategoryId id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawSystem> drawSystem);

    const std::string & getName() const;
    void setName(const std::string &name);

    const CategoryId & getId() const;

    const std::map<MatchId, std::unique_ptr<MatchStore>> & getMatches() const;
    MatchStore & getMatch(MatchId id);
    const MatchStore & getMatch(MatchId id) const;
    std::unique_ptr<MatchStore> eraseMatch(MatchId id);
    void addMatch(std::unique_ptr<MatchStore> && ptr);
    bool containsMatch(MatchId id) const;

    const std::unordered_set<PlayerId, PlayerId::Hasher> & getPlayers() const;
    void erasePlayer(PlayerId id);
    void addPlayer(PlayerId id);
    bool containsPlayer(PlayerId id) const;

    void setRuleset(std::unique_ptr<Ruleset> && ptr);
    Ruleset & getRuleset();
    const Ruleset & getRuleset() const;

    void setDrawSystem(std::unique_ptr<DrawSystem> && ptr);
    DrawSystem & getDrawSystem();
    const DrawSystem & getDrawSystem() const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("id", mId));
        ar(cereal::make_nvp("name", mName));
        ar(cereal::make_nvp("players", mPlayers));
        ar(cereal::make_nvp("matches", mMatches));
        ar(cereal::make_nvp("ruleset", mRuleset));
        ar(cereal::make_nvp("drawSystem", mDrawSystem));
    }
private:
    CategoryId mId;
    std::string mName;
    std::unordered_set<PlayerId, PlayerId::Hasher> mPlayers;
    std::map<MatchId, std::unique_ptr<MatchStore>> mMatches; // order matters in this case
    std::unique_ptr<Ruleset> mRuleset;
    std::unique_ptr<DrawSystem> mDrawSystem;
};

