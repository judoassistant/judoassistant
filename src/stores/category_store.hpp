#pragma once

#include <string>
#include <unordered_set>
#include <map>

#include "core.hpp"
#include "stores/match_store.hpp"
#include "rulesets/ruleset.hpp"

class CategoryStore {
public:
    CategoryStore();

    const std::unordered_set<Id> & getPlayers() const;
    const std::string & getName() const;
    const Id & getId() const;

    const std::map<Id, std::unique_ptr<MatchStore>> & getMatches() const;
    std::unique_ptr<MatchStore> & getMatch(Id id);

    void addMatch(std::unique_ptr<MatchStore> && ptr);

    void setRuleset(std::unique_ptr<Ruleset> && ptr);
    std::unique_ptr<Ruleset> & getRuleset();
private:
    Id mId;
    std::string mName;
    std::unordered_set<Id> mPlayers;
    std::map<Id, std::unique_ptr<MatchStore>> mMatches; // order matters in this case
    std::unique_ptr<Ruleset> mRuleset;
};

