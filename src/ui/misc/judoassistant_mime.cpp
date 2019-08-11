#include <QByteArray>

#include "ui/misc/judoassistant_mime.hpp"

JudoassistantMime::JudoassistantMime()
    : QMimeData()
{

}

void JudoassistantMime::setBlock(CategoryId categoryId, MatchType matchType) {
    QByteArray ba;
    ba.append(reinterpret_cast<char*>(&categoryId), sizeof(categoryId));
    ba.append(reinterpret_cast<char*>(&matchType), sizeof(matchType));
    setData("application/judoassistant-block", ba);
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
