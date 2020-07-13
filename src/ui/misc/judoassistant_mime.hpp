#pragma once

#include <QMimeData>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/stores/match_store.hpp"

class JudoassistantMime : public QMimeData {
    Q_OBJECT
public:
    JudoassistantMime();

    std::pair<CategoryId,MatchType> block() const;
    void setBlock(CategoryId categoryId, MatchType matchType);
    bool hasBlock() const;
    void setText(const QString &categoryName, MatchType matchType);

private:
};
