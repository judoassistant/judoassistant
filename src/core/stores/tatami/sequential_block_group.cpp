#include "core/log.hpp"
#include "core/stores/tatami/sequential_block_group.hpp"
#include "core/stores/tatami/location.hpp"
#include "core/stores/category_store.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/match_store.hpp"
#include "core/stores/tournament_store.hpp"

SequentialBlockGroup::SequentialBlockGroup()
    : mExpectedDuration(std::chrono::milliseconds(0))
{}

void SequentialBlockGroup::eraseBlock(std::pair<CategoryId, MatchType> block) {
    auto it = std::find(mBlocks.begin(), mBlocks.end(), block);
    assert(it != mBlocks.end());
    mBlocks.erase(it);
}

void SequentialBlockGroup::addBlock(size_t index, std::pair<CategoryId, MatchType> block) {
    assert(index <= mBlocks.size());
    mBlocks.insert(mBlocks.begin() + index, block);
}

size_t SequentialBlockGroup::blockCount() const {
    return mBlocks.size();
}

const std::pair<CategoryId, MatchType> & SequentialBlockGroup::at(size_t index) const {
    assert(blockCount() > index);
    return mBlocks[index];
}

std::pair<CategoryId, MatchType> & SequentialBlockGroup::at(size_t index) {
    assert(blockCount() > index);
    return mBlocks[index];
}

size_t SequentialBlockGroup::getIndex(std::pair<CategoryId, MatchType> block) const {
    auto it = std::find(mBlocks.begin(), mBlocks.end(), block);
    assert (it != mBlocks.end());
    return std::distance(mBlocks.begin(), it);
}

ConstMatchIterator SequentialBlockGroup::matchesBegin(const TournamentStore &tournament) const {
    return ConstMatchIterator(tournament, *this, 0, 0);
}

ConstMatchIterator SequentialBlockGroup::matchesEnd(const TournamentStore &tournament) const {
    return ConstMatchIterator(tournament, *this, mBlocks.size(), 0);
}

void SequentialBlockGroup::recompute(const TournamentStore &tournament) {
    mExpectedDuration = std::chrono::seconds(0);

    for (size_t i = 0; i < mBlocks.size(); ++i) {
        const auto block = mBlocks[i];
        const auto &category = tournament.getCategory(block.first);

        mExpectedDuration += category.expectedDuration(block.second);
    }
}

std::chrono::milliseconds SequentialBlockGroup::getExpectedDuration() const {
    return mExpectedDuration;
}

