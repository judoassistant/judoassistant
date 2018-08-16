#pragma once

#include "core.hpp"
#include <vector>
#include "stores/category_store.hpp"
#include "actions/action.hpp"

class DrawStrategy {
public:
    virtual void initializeCategory(const CategoryStore & category, std::vector<std::unique_ptr<Action>> * actions) = 0;
    virtual void updateCategory(const CategoryStore & category, std::vector<std::unique_ptr<Action>> * actions) = 0;
    virtual bool isFinished(const CategoryStore & category) = 0;
    virtual void computeRanking(const CategoryStore & category, std::vector<std::pair<size_t, Id>> *ranking) = 0;
};

