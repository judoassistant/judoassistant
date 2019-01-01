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

class TatamiList {
public:
    void eraseTatami(PositionHandle handle);
    bool containsTatami(PositionHandle handle) const;
    bool containsTatami(TatamiLocation location) const;
    PositionHandle getHandle(size_t index) const;
    size_t getIndex(PositionHandle handle) const;
    size_t getIndex(TatamiLocation location) const;
    size_t tatamiCount() const;
    TatamiLocation addTatami(size_t index);
    BlockLocation refreshLocation(BlockLocation location, std::pair<CategoryId, MatchType> block) const;

    TatamiStore & operator[](PositionHandle handle);

    const TatamiStore & at(PositionHandle handle) const;
    TatamiStore & at(PositionHandle handle);
    const TatamiStore & at(size_t index) const;
    TatamiStore & at(TatamiLocation location);
    const TatamiStore & at(TatamiLocation location) const;
    const ConcurrentBlockGroup & at(ConcurrentGroupLocation location) const;
    const SequentialBlockGroup & at(SequentialGroupLocation location) const;

    void moveBlock(const TournamentStore &tournament, std::pair<CategoryId, MatchType> block, std::optional<BlockLocation> from, std::optional<BlockLocation> to);
    void recomputeBlock(const TournamentStore &tournament, BlockLocation);

    // std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>> popTatami();
    // void recoverTatami(const std::vector<std::tuple<CategoryId, MatchType, TatamiLocation>> &contents);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mTatamis);
    }

private:
    PositionManager<TatamiStore> mTatamis;
};

