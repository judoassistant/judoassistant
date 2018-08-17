#pragma once

#include "src/draw_strategies/draw_strategy.hpp"

class PoolDrawStrategy : public DrawStrategy {
public:
    PoolDrawStrategy();

    virtual void initCategory(const std::vector<Id> &players, TournamentStore *tournament, CategoryStore *category) = 0;
    virtual void updateCategory(TournamentStore *tournament, CategoryStore *categoryStore) = 0;
    virtual bool isFinished() = 0;
    virtual Id get_rank(size_t rank) = 0;
};
