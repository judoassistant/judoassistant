#pragma once

#include "draw_strategies/draw_strategy.hpp"
#include "serialize.hpp"
#include "draw_strategies/draw_strategy.hpp"

class PoolDrawStrategy : public DrawStrategy {
public:
    PoolDrawStrategy() {}

    void initCategory(StoreHandler & store_handler, const std::vector<Id> &players, TournamentStore & tournament, CategoryStore & category) override;
    void updateCategory(StoreHandler & store_handler, TournamentStore & tournament, CategoryStore & category) override;
    bool isFinished(TournamentStore & tournament, CategoryStore & category) const override;
    Id get_rank(size_t rank) const override;
    std::unique_ptr<DrawStrategy> clone() const override;
    std::string getName() const override;
    static std::string getStaticName();
private:
    std::vector<Id> mMatchIds;
};
