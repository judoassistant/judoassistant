#pragma once

#include "core.hpp"
#include "id.hpp"
#include "hash.hpp"
#include "position_manager.hpp"

enum class MatchType;

struct TatamiLocation {
    size_t tatamiIndex;
    PositionHandle concurrentGroup;
    PositionHandle sequentialGroup;
};

class TournamentStore;
class CategoryStore;

class SequentialBlockGroup {
public:
    // Both random insertion/deletion and access must be supported. We use a vector since the size stays small
    typedef std::vector<std::pair<CategoryId, MatchType>> BlockList;

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

    ConstMatchIterator matchesBegin(const TournamentStore &tournament) const;
    ConstMatchIterator matchesEnd(const TournamentStore &tournament) const;
    size_t getMatchCount() const;

    std::pair<CategoryId, MatchType> getBlock(size_t pos) const;
    void addBlock(size_t pos, CategoryId categoryId, MatchType type);
    void eraseBlock(CategoryId categoryId, MatchType type);
    size_t findBlock(CategoryId categoryId, MatchType type);
    size_t blockCount() const;

    void recompute(const TournamentStore &tournament);
private:
    BlockList mBlocks;
    size_t mMatchCount;
};

class ConcurrentBlockGroup {
public:
    typedef std::vector<std::pair<CategoryId, MatchId>> MatchList; // Iteration and swapping of two elements given indexes must be supported

    ConcurrentBlockGroup();

    // TODO: Add interface to delay matches
    const MatchList & getMatches() const;

    void eraseGroup(PositionHandle handle);
    PositionHandle addGroup(TournamentStore & tournament, size_t index);
    PositionHandle getHandle(size_t index) const;
    size_t groupCount() const;

    SequentialBlockGroup & getGroup(PositionHandle handle);

    void recompute(const TournamentStore &tournament);
private:
    PositionManager<SequentialBlockGroup> mGroups;
    MatchList mMatches;
    std::unordered_map<std::pair<CategoryId, MatchId>,size_t> mMatchMap;
    std::unordered_map<std::pair<CategoryId, MatchType>,size_t> mCategoryMap;
};

class TatamiStore {
public:
    void eraseGroup(PositionHandle handle);
    PositionHandle addGroup(TournamentStore & tournament, size_t index);
    PositionHandle getHandle(size_t index) const;
    size_t groupCount() const;

    ConcurrentBlockGroup & getGroup(PositionHandle handle);
private:
    PositionManager<ConcurrentBlockGroup> mGroups;
    std::unordered_map<std::pair<CategoryId, MatchType>,size_t> mCategoryMap;
};

class TatamiList {
public:
    size_t getSeqIndex(TatamiLocation location, CategoryId categoryId, MatchType type);
    bool containsTatami(TatamiLocation location);
    void recomputeBlock(const TournamentStore &tournament, TatamiLocation location);
    void moveBlock(const TournamentStore &tournament, CategoryId categoryId, MatchType type, std::optional<TatamiLocation> from, std::optional<TatamiLocation> to, size_t seqIndex);

    void pushTatami();
    std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>> popTatami();
    void recoverTatami(const std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>> &contents);
    size_t tatamiCount() const;
private:
    std::vector<TatamiStore> mTatamis;
};

