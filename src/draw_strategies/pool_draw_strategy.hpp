#pragma once

#include "draw_strategies/draw_strategy.hpp"
#include "serialize.hpp"

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

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("matchIds", mMatchIds));
    }
private:
    std::vector<Id> mMatchIds;
};

CEREAL_REGISTER_TYPE(PoolDrawStrategy)
CEREAL_REGISTER_POLYMORPHIC_RELATION(DrawStrategy, PoolDrawStrategy)
