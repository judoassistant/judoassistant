#include "core/stores/category_store.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tatami/const_match_iterator.hpp"
#include "core/stores/tatami/sequential_block_group.hpp"
#include "core/stores/tournament_store.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/draw_systems/draw_system.hpp"

ConstMatchIterator & ConstMatchIterator::operator++() {
    ++mCurrentMatch;
    loadMatch();

    return *this;
}

CombinedId ConstMatchIterator::operator*() {
    assert(mCurrentCategory != nullptr);
    auto matchId = mCurrentCategory->getMatches()[mCurrentMatch]->getId();
    return CombinedId(mCurrentCategory->getId(), matchId);
}

bool ConstMatchIterator::operator!=(const ConstMatchIterator &other) const {
    return !(*this == other);
}

bool ConstMatchIterator::operator==(const ConstMatchIterator &other) const {
    return mCurrentBlock == other.mCurrentBlock && mCurrentMatch == other.mCurrentMatch;
}

ConstMatchIterator::ConstMatchIterator(const TournamentStore &tournament, const SequentialBlockGroup &group, size_t currentBlock, size_t currentMatch)
    : mTournament(tournament)
    , mGroup(group)
    , mCurrentCategory(nullptr)
    , mCurrentBlock(currentBlock)
    , mCurrentMatch(currentMatch)
{
    loadMatch();
}

void ConstMatchIterator::loadMatch() {
    while (true) {
        if (mCurrentBlock == mGroup.blockCount())
            break;

        if (mCurrentCategory == nullptr) {
            auto block = mGroup.at(mCurrentBlock);
            mCurrentCategory = &mTournament.getCategory(block.first);
            mCurrentType = block.second;
        }

        if (mCurrentMatch == mCurrentCategory->getMatches().size()) {
            mCurrentCategory = nullptr;
            mCurrentMatch = 0;
            ++mCurrentBlock;
            continue;
        }

        const auto &match = *(mCurrentCategory->getMatches()[mCurrentMatch]);
        if (match.isPermanentBye()) {
            ++mCurrentMatch;
            continue;
        }

        if (mCurrentType != match.getType()) {
            ++mCurrentMatch;
            continue;
        }

        break;
    }
}

