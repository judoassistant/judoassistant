#pragma once

#include <string>
#include <unordered_set>
#include <map>

#include "core.hpp"
#include "stores/match_store.hpp"
#include "draw_strategies/draw_strategy.hpp"
#include "rulesets/ruleset.hpp"

class CategoryStore {
public:
    CategoryStore() {}
    CategoryStore(Id id, const std::string &name, std::unique_ptr<Ruleset> ruleset, std::unique_ptr<DrawStrategy> drawStrategy);

    const std::unordered_set<Id> & getPlayers() const;
    const std::string & getName() const;
    const Id & getId() const;

    const std::map<Id, std::unique_ptr<MatchStore>> & getMatches() const;
    MatchStore & getMatch(Id id);
    std::unique_ptr<MatchStore> eraseMatch(Id id);

    void addMatch(std::unique_ptr<MatchStore> && ptr);

    void setRuleset(std::unique_ptr<Ruleset> && ptr);
    Ruleset & getRuleset();

    void setDrawStrategy(std::unique_ptr<DrawStrategy> && ptr);
    DrawStrategy & getDrawStrategy();

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("id", mId));
        ar(cereal::make_nvp("name", mName));
        ar(cereal::make_nvp("players", mPlayers));
        ar(cereal::make_nvp("matches", mMatches));
        ar(cereal::make_nvp("ruleset", mRuleset));
        ar(cereal::make_nvp("drawStrategy", mDrawStrategy));
    }
private:
    Id mId;
    std::string mName;
    std::unordered_set<Id> mPlayers;
    std::map<Id, std::unique_ptr<MatchStore>> mMatches; // order matters in this case
    std::unique_ptr<Ruleset> mRuleset;
    std::unique_ptr<DrawStrategy> mDrawStrategy;
};

