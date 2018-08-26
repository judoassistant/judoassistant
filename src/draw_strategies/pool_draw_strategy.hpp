#pragma once

#include "draw_strategies/draw_strategy.hpp"
#include "serialize.hpp"

class PoolDrawStrategy : public DrawStrategy {
public:
    PoolDrawStrategy();

    void initCategory(const std::vector<Id> &players, TournamentStore & tournament, CategoryStore & category) override;
    void updateCategory(TournamentStore & tournament, CategoryStore & category) override;
    bool isFinished(TournamentStore & tournament, CategoryStore & category) const override;
    Id get_rank(size_t rank) const override;
private:
    std::vector<Id> mMatchIds;
};
