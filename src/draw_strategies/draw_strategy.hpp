#pragma once

#include "core.hpp"
#include <vector>
#include "actions/action.hpp"
#include "serialize.hpp"

class StoreHandler;
class CategoryStore;
class TournamentStore;

// TODO: Make const where appropriate
class DrawStrategy {
public:
    virtual ~DrawStrategy() {};
    virtual void initCategory(StoreHandler & store_handler, const std::vector<Id> &players, TournamentStore & tournament, CategoryStore & category) = 0;
    virtual void updateCategory(StoreHandler & store_handler, TournamentStore & tournament, CategoryStore & category) = 0;
    virtual bool isFinished(TournamentStore & tournament, CategoryStore & category) const = 0;
    virtual Id get_rank(size_t rank) const = 0; // TODO: change this interface to handle players sharing rank
};

// TODO: somehow handle undoing of winning scores (remove matches after they have been created)

