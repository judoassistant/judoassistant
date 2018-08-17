#pragma once

#include <optional>

#include "core.hpp"

class MatchStore {
public:
    MatchStore(Id id, std::optional<Id> whitePlayer, std::optional<Id> bluePlayer);

    Id getId() const;
    std::optional<Id> getWhitePlayer() const;
    std::optional<Id> getBluePlayer() const;
    bool isFinished() const;
private:
    Id mId;
    std::optional<Id> mWhitePlayer;
    std::optional<Id> mBluePlayer;
};

