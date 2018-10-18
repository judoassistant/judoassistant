#include "widgets/misc/qutejudo_mime.hpp"
#include <QByteArray>

QutejudoMime::QutejudoMime()
    : QMimeData()
{

}

void QutejudoMime::setBlock(CategoryId categoryId, MatchType matchType) {
    std::optional<std::pair<CategoryId, MatchType>> mBlock;
    QByteArray ba;
    ba.append(reinterpret_cast<char*>(&categoryId), sizeof(categoryId));
    ba.append(reinterpret_cast<char*>(&matchType), sizeof(matchType));
    setData("application/qutejudo-block", ba);
}

bool QutejudoMime::hasBlock() const {
    return hasFormat("application/qutejudo-block");
}

std::pair<CategoryId,MatchType> QutejudoMime::block() const {
    QByteArray ba = data("application/qutejudo-block");
    CategoryId categoryId = *(reinterpret_cast<CategoryId*>(ba.data()));
    MatchType matchType = *(reinterpret_cast<MatchType*>(ba.data() + sizeof(CategoryId)));

    return {categoryId, matchType};
}
