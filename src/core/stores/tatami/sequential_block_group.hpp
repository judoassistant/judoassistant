#pragma once

#include <vector>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"

enum class MatchType;
class TournamentStore;
struct BlockLocation;
class ConstMatchIterator;

class SequentialBlockGroup {
public:
    SequentialBlockGroup();

    void eraseBlock(std::pair<CategoryId, MatchType> block);
    void addBlock(size_t index, std::pair<CategoryId, MatchType> block);
    size_t blockCount() const;

    std::pair<CategoryId, MatchType> & at(size_t index);
    const std::pair<CategoryId, MatchType> & at(size_t index) const;
    size_t getIndex(std::pair<CategoryId, MatchType> block) const;

    ConstMatchIterator matchesBegin(const TournamentStore &tournament) const;
    ConstMatchIterator matchesEnd(const TournamentStore &tournament) const;

    void recompute(const TournamentStore &tournament);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mBlocks, mExpectedDuration);
    }

    std::chrono::milliseconds getExpectedDuration() const;
private:
    std::vector<std::pair<CategoryId, MatchType>> mBlocks;
    std::chrono::milliseconds mExpectedDuration;
};

