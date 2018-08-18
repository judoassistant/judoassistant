#pragma once

#include "core.hpp"

class TournamentStore;

class Action {
public:
    virtual bool operator()(TournamentStore * store) const = 0;
    virtual std::unique_ptr<Action> getInverse() const = 0;
};

