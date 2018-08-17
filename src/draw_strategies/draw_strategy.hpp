#pragma once

#include "core.hpp"
#include <vector>
#include "stores/category_store.hpp"
#include "actions/action.hpp"

class DrawStrategy {
public:
    virtual void initCategory(const std::vector<Id> &players, TournamentStore *tournament, CategoryStore *category) = 0;
    virtual void updateCategory(TournamentStore *tournament, CategoryStore *categoryStore) = 0;
    virtual bool isFinished(TournamentStore *tournament, CategoryStore *categoryStore) = 0;
    virtual Id get_rank(size_t rank) = 0;
};

