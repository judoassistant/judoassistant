#pragma once

#include <vector>

#include "core.hpp"
#include "id.hpp"
#include "serialize.hpp"

class Action;
class CategoryStore;
class TournamentStore;

class DrawSystem {
public:
    virtual ~DrawSystem() {};

    virtual std::unique_ptr<DrawSystem> clone() const = 0;
    virtual std::string getName() const = 0;
    virtual bool hasFinalBlock() const = 0;

    virtual std::vector<std::unique_ptr<Action>> initCategory(const std::vector<PlayerId> &playerIds, const TournamentStore &tournament, const CategoryStore &category) = 0;
    virtual std::vector<std::unique_ptr<Action>> updateCategory(const TournamentStore & tournament, const CategoryStore & category) const = 0;
    virtual bool isFinished(const TournamentStore &tournament, const CategoryStore &category) const = 0;
    virtual std::vector<std::pair<std::optional<unsigned int>, PlayerId>> getResults(const TournamentStore &tournament, const CategoryStore &category) const = 0;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}
};

