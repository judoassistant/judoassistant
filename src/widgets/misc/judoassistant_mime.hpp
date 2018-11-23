#pragma once

#include <QMimeData>
#include "core.hpp"
#include "id.hpp"
#include "stores/match_store.hpp"

class JudoassistantMime : public QMimeData {
    Q_OBJECT
public:
    JudoassistantMime();

    std::pair<CategoryId,MatchType> block() const;
    void setBlock(CategoryId categoryId, MatchType matchType);
    bool hasBlock() const;
};
