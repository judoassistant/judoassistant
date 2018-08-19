#pragma once

#include "core.hpp"
#include <vector>
#include "stores/category_store.hpp"
#include "actions/action.hpp"

// TODO: Make const where appropriate
class DrawStrategy {
public:
    virtual void initCategory(const std::vector<Id> &players, std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) = 0;
    virtual void updateCategory(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) = 0;
    virtual bool isFinished(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) const = 0;
    virtual Id get_rank(size_t rank) const = 0; // TODO: change this interface to handle players sharing rank
};

