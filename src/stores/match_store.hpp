#pragma once

#include <optional>

#include "core.hpp"

class MatchStore {
public:
    MatchStore(Id id, Id category, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer);

    const Id & getId() const;
    const Id & getCategory() const;
    const std::optional<Id> & getWhitePlayer() const;
    const std::optional<Id> & getBluePlayer() const;
private:
    Id mId;
    Id mCategory;
    std::optional<Id> mWhitePlayer;
    std::optional<Id> mBluePlayer;
};

