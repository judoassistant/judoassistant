#pragma once

#include "core.hpp"
#include "id.hpp"
#include "hash.hpp"
#include "position_manager.hpp"
#include "serialize.hpp"

enum class MatchType;

struct TatamiLocation {
    size_t tatamiIndex;
    PositionHandle concurrentGroup;
    PositionHandle sequentialGroup;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("tatamiIndex", tatamiIndex));
        ar(cereal::make_nvp("concurrentGroup", concurrentGroup));
        ar(cereal::make_nvp("sequentialGroup", sequentialGroup));
    }

    bool operator==(const TatamiLocation &other) const {
        return tatamiIndex == other.tatamiIndex
            && concurrentGroup == other.concurrentGroup
            && sequentialGroup == other.sequentialGroup;
    }
};

std::ostream &operator<<(std::ostream &out, const TatamiLocation &location);

namespace std {
    template <>
    struct hash<TatamiLocation> {
        size_t operator()(const TatamiLocation &v) const {
            size_t seed = 0;
            hash_combine(seed, v.tatamiIndex);
            hash_combine(seed, v.concurrentGroup);
            hash_combine(seed, v.sequentialGroup);
            return seed;
        }
    };
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

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("blocks", mBlocks));
        ar(cereal::make_nvp("matchCount", mMatchCount));
    }
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

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("groups", mGroups));
        ar(cereal::make_nvp("matches", mMatches));
        ar(cereal::make_nvp("matchMap", mMatchMap));
    }
private:
    PositionManager<SequentialBlockGroup> mGroups;
    MatchList mMatches;
    std::unordered_map<std::pair<CategoryId, MatchId>,size_t> mMatchMap;
};

class TatamiStore {
public:
    void eraseGroup(PositionHandle handle);
    std::pair<PositionHandle, PositionHandle> addGroup(TournamentStore & tournament, size_t index);
    bool containsGroup(PositionHandle handle) const;
    PositionHandle getHandle(size_t index) const;
    size_t groupCount() const;

    ConcurrentBlockGroup & getGroup(PositionHandle handle);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("groups", mGroups));
    }
private:
    PositionManager<ConcurrentBlockGroup> mGroups;
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
    TatamiStore & operator[](size_t index);
    const TatamiStore & operator[](size_t index) const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(cereal::make_nvp("tatamis", mTatamis));
    }
private:
    std::vector<TatamiStore> mTatamis;
};

