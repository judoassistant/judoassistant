#pragma once

#include "core/core.hpp"
#include "core/id.hpp"

class SequentialBlockGroup;
enum class MatchType;

// Iterates non-hidden matches in a sequential block group
class ConstMatchIterator {
public:
    ConstMatchIterator & operator++();
    CombinedId operator*();
    bool operator!=(const ConstMatchIterator &other) const;
    bool operator==(const ConstMatchIterator &other) const;

private:
    ConstMatchIterator(const TournamentStore &tournament, const SequentialBlockGroup &group, size_t currentBlock, size_t currentMatch);
    void loadMatch();

    const TournamentStore &mTournament;
    const SequentialBlockGroup &mGroup;
    const CategoryStore *mCurrentCategory;
    MatchType mCurrentType;
    size_t mCurrentBlock;
    size_t mCurrentMatch;

    friend class SequentialBlockGroup;
};

