#include "ui/misc/category_name_comparator.hpp"

CategoryNameComparator::CategoryNameComparator(const TournamentStore &tournament)
    : mComp()
    , mTournament(tournament)
{}

bool CategoryNameComparator::operator()(const CategoryId &a, const CategoryId &b) const {
    const auto &first = mTournament.getCategory(a);
    const auto &second = mTournament.getCategory(b);
    return mComp(QString::fromStdString(first.getName()), QString::fromStdString(second.getName()));
}

