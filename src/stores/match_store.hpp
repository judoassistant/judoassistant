#pragma once

#include <string>
#include <cstdint>
#include <unordered_set>

#include "core.hpp"

class MatchStore {
public:
    MatchStore(Id id, Id category, Id whitePlayer, Id bluePlayer);

    const Id & getId() const;
    const Id & getCategory() const;
    const Id & getWhitePlayer() const;
    const Id & getBluePlayer() const;
private:
    Id mId;
    Id mCategory;
    Id mWhitePlayer;
    Id mBluePlayer;
};

