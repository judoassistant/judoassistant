#pragma once

#include <QMimeData>
#include "core.hpp"
#include "id.hpp"
#include "stores/match_store.hpp"

class QutejudoMime : public QMimeData {
    Q_OBJECT
public:
    QutejudoMime();

    std::pair<CategoryId,MatchType> block() const;
    void setBlock(CategoryId categoryId, MatchType matchType);
    bool hasBlock() const;
};
