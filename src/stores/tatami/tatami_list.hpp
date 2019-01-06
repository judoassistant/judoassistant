#pragma once

#include "core.hpp"
#include "id.hpp"
#include "position_manager.hpp"
#include "serialize.hpp"
#include "stores/tatami/tatami_store.hpp"

enum class MatchType;
class TournamentStore;
class TatamiLocation;
class ConcurrentGroupLocation;
class SequentialGroupLocation;

// TODO: Refactor to avoid public methods accepting PositionHandle
class TatamiList {
public:
    void eraseTatami(PositionHandle handle);
    bool containsTatami(PositionHandle handle) const;
    bool containsTatami(TatamiLocation location) const;
    PositionHandle getHandle(size_t index) const;
    size_t getIndex(PositionHandle handle) const;
    size_t getIndex(TatamiLocation location) const;
    size_t tatamiCount() const;
    TatamiLocation generateLocation(size_t index);
    BlockLocation refreshLocation(BlockLocation location, std::pair<CategoryId, MatchType> block) const;
    void insert(PositionHandle handle);

    TatamiStore & operator[](PositionHandle handle);

    const TatamiStore & at(PositionHandle handle) const;
    TatamiStore & at(PositionHandle handle);

    const TatamiStore & at(size_t index) const;
    TatamiStore & at(size_t index);

    TatamiStore & at(TatamiLocation location);
    const TatamiStore & at(TatamiLocation location) const;

    const ConcurrentBlockGroup & at(ConcurrentGroupLocation location) const;
    ConcurrentBlockGroup & at(ConcurrentGroupLocation location);

    const SequentialBlockGroup & at(SequentialGroupLocation location) const;
    SequentialBlockGroup & at(SequentialGroupLocation location);

    void moveBlock(const TournamentStore &tournament, std::pair<CategoryId, MatchType> block, std::optional<BlockLocation> from, std::optional<BlockLocation> to);
    void recomputeBlock(const TournamentStore &tournament, BlockLocation);
    void updateStatus(const TournamentStore &tournament, BlockLocation location);

    // std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>> popTatami();
    // void recoverTatami(const std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>> &contents);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mTatamis);
    }

private:
    PositionManager<TatamiStore> mTatamis;
};

