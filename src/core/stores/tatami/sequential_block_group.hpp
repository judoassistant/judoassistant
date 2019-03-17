#pragma once

#include <vector>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"

enum class MatchType;
class TournamentStore;
class BlockLocation;

class SequentialBlockGroup {
public:
    static constexpr std::chrono::milliseconds MIN_EXPECTED_DURATION = std::chrono::minutes(15);

    class ConstMatchIterator {
    public:
        ConstMatchIterator & operator++();
        std::pair<CategoryId, MatchId> operator*();
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

    SequentialBlockGroup();

    void eraseBlock(std::pair<CategoryId, MatchType> block);
    void addBlock(size_t index, std::pair<CategoryId, MatchType> block);
    size_t blockCount() const;

    std::pair<CategoryId, MatchType> & at(size_t index);
    const std::pair<CategoryId, MatchType> & at(size_t index) const;
    size_t getIndex(std::pair<CategoryId, MatchType> block) const;

    ConstMatchIterator matchesBegin(const TournamentStore &tournament) const;
    ConstMatchIterator matchesEnd(const TournamentStore &tournament) const;
    size_t getMatchCount() const;

    void recompute(const TournamentStore &tournament);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mBlocks, mMatchCount, mExpectedDuration);
    }

    std::chrono::milliseconds getExpectedDuration() const;
private:
    std::vector<std::pair<CategoryId, MatchType>> mBlocks;
    size_t mMatchCount;
    std::chrono::milliseconds mExpectedDuration;
};

