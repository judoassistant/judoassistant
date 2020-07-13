#include <QByteArray>

#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "ui/misc/judoassistant_mime.hpp"

JudoassistantMime::JudoassistantMime()
    : QMimeData()
{

}

void JudoassistantMime::setBlock(CategoryId categoryId, MatchType matchType) {
    QByteArray ba;
    ba.append(reinterpret_cast<char*>(&categoryId), sizeof(CategoryId));
    ba.append(reinterpret_cast<char*>(&matchType), sizeof(MatchType));
    setData("application/judoassistant-block", ba);
}

void JudoassistantMime::setText(const QString &categoryName, MatchType matchType) {
    QString text = categoryName;

    text += ' ';
    if (matchType == MatchType::ELIMINATION)
        text += tr("(Elimination)");
    else
        text += tr("(Final)");

    QMimeData::setText(text);
}

bool JudoassistantMime::hasBlock() const {
    return hasFormat("application/judoassistant-block");
}

std::pair<CategoryId,MatchType> JudoassistantMime::block() const {
    QByteArray ba = data("application/judoassistant-block");
    CategoryId categoryId = *(reinterpret_cast<CategoryId*>(ba.data()));
    MatchType matchType = *(reinterpret_cast<MatchType*>(ba.data() + sizeof(CategoryId)));

    return {categoryId, matchType};
}
