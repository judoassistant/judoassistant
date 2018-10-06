#pragma once

#include "core.hpp"
#include <vector>
#include "actions/action.hpp"
#include "serialize.hpp"

class StoreHandler;
class CategoryStore;
class TournamentStore;

// TODO: Make const where appropriate
// TODO: Consider renaming to DrawSystemStrategy to increase consistency with UI naming
class DrawStrategy {
public:
    virtual ~DrawStrategy() {};
    virtual void initCategory(StoreHandler & store_handler, const std::vector<Id> &players, TournamentStore & tournament, CategoryStore & category) = 0;
    virtual void updateCategory(StoreHandler & store_handler, TournamentStore & tournament, CategoryStore & category) = 0;
    virtual bool isFinished(TournamentStore & tournament, CategoryStore & category) const = 0;
    virtual Id get_rank(size_t rank) const = 0; // TODO: change this interface to handle players sharing rank
    virtual std::unique_ptr<DrawStrategy> clone() const = 0;
    virtual std::string getName() const = 0;
};

// TODO: somehow handle undoing of winning scores (remove matches after they have been created)

