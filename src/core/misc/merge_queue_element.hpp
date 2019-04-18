#pragma once

// Struct used for merging list of matches evenly
struct MergeQueueElement {
    MergeQueueElement(size_t index, size_t matchCount, size_t totalMatchCount)
        : index(index)
        , matchCount(matchCount)
        , totalMatchCount(totalMatchCount)
    {}

    bool operator<(const MergeQueueElement &other) const {
        // First order by fraction (desc), then total match count(asc) then index(asc)
        // Avoiding floating point errors.
        // mMatchCount/mTotalMatchCount > other.mMatchCount/other.mTotalMatchCount
        if (matchCount * other.totalMatchCount != other.matchCount * totalMatchCount)
            return (matchCount * other.totalMatchCount) > (other.matchCount * totalMatchCount);
        if (totalMatchCount < other.totalMatchCount)
            return totalMatchCount < other.totalMatchCount;
        return index < other.index;
    }

    size_t index;
    size_t matchCount;
    size_t totalMatchCount;
};

