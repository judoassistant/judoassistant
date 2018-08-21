#pragma once

#include "draw_strategies/draw_strategy.hpp"
#include "serialize.hpp"

class PoolDrawStrategy : public DrawStrategy {
public:
    PoolDrawStrategy();

    virtual void initCategory(const std::vector<Id> &players, std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) = 0;
    virtual void updateCategory(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category);
    virtual bool isFinished(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) const;
    virtual Id get_rank(size_t rank) const;
private:
    std::vector<Id> mMatchIds;
};
