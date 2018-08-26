#pragma once

#include "draw_strategies/draw_strategy.hpp"
#include "serialize.hpp"

class PoolDrawStrategy : public DrawStrategy {
public:
    PoolDrawStrategy();

    void initCategory(const std::vector<Id> &players, std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) override;
    void updateCategory(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) override;
    bool isFinished(std::unique_ptr<TournamentStore> & tournament, std::unique_ptr<CategoryStore> & category) const override;
    Id get_rank(size_t rank) const override;
private:
    std::vector<Id> mMatchIds;
};
