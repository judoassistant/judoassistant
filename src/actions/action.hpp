#pragma once

#include "stores/tournament_store.hpp"

class Action {
public:
    virtual bool operator()(TournamentStore * store) const = 0;
    virtual void getInverse(Action *ptr) const = 0;
};

