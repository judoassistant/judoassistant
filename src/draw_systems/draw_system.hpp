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

    virtual std::vector<std::unique_ptr<Action>> initCategory(const std::vector<PlayerId> &playerIds, TournamentStore & tournament, CategoryStore & category) = 0;
    virtual std::vector<std::unique_ptr<Action>> updateCategory(TournamentStore & tournament, CategoryStore & category) const = 0;
    virtual bool isFinished(TournamentStore & tournament, CategoryStore & category) const = 0;
    virtual std::vector<std::pair<size_t, PlayerId>> get_results() const = 0;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}
};

