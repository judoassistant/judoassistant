#include <chrono>
#include <set>

#include "core/id.hpp"
#include "core/hash.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

MatchId MatchId::generate(const CategoryStore &category, MatchId::Generator &generator) {
    MatchId id;
    do {
        id = generator();
    } while(category.containsMatch(id));

    return id;
}

MatchId MatchId::generate(const CategoryStore &category) {
    static std::unique_ptr<MatchId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<MatchId::Generator>(getSeed());

    return generate(category, *generator);
}

std::vector<MatchId> MatchId::generateMultiple(const CategoryStore &category, MatchId::Generator &generator, size_t count) {
    std::set<MatchId> ids;
    while (ids.size() < count) {
        ids.insert(generate(category, generator));
    }

    return std::vector(ids.begin(), ids.end());
}

std::vector<MatchId> MatchId::generateMultiple(const CategoryStore &category, size_t count) {
    std::set<MatchId> ids;
    while (ids.size() < count) {
        ids.insert(generate(category));
    }

    return std::vector(ids.begin(), ids.end());
}

PlayerId PlayerId::generate(const TournamentStore &tournament, PlayerId::Generator &generator) {
    PlayerId id;
    do {
        id = generator();
    } while(tournament.containsPlayer(id));

    return id;
}

PlayerId PlayerId::generate(const TournamentStore &tournament) {
    static std::unique_ptr<PlayerId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<PlayerId::Generator>(getSeed());

    return generate(tournament, *generator);
}

std::vector<PlayerId> PlayerId::generateMultiple(const TournamentStore &tournament, const size_t count) {
    std::set<PlayerId> ids;
    while (ids.size() < count) {
        ids.insert(generate(tournament));
    }

    return std::vector(ids.begin(), ids.end());
}

std::vector<PlayerId> PlayerId::generateMultiple(const TournamentStore &tournament, PlayerId::Generator &generator, const size_t count) {
    std::set<PlayerId> ids;
    while (ids.size() < count) {
        ids.insert(generate(tournament, generator));
    }

    return std::vector(ids.begin(), ids.end());
}

CategoryId CategoryId::generate(const TournamentStore &tournament, CategoryId::Generator &generator) {
    CategoryId id;
    do {
        id = generator();
    } while(tournament.containsCategory(id));

    return id;
}

CategoryId CategoryId::generate(const TournamentStore &tournament) {
    static std::unique_ptr<CategoryId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<CategoryId::Generator>(getSeed());

    return generate(tournament, *generator);
}

std::vector<CategoryId> CategoryId::generateMultiple(const TournamentStore &tournament, const size_t count) {
    std::set<CategoryId> ids;
    while (ids.size() < count) {
        ids.insert(generate(tournament));
    }

    return std::vector(ids.begin(), ids.end());
}

std::vector<CategoryId> CategoryId::generateMultiple(const TournamentStore &tournament, CategoryId::Generator &generator, const size_t count) {
    std::set<CategoryId> ids;
    while (ids.size() < count) {
        ids.insert(generate(tournament, generator));
    }

    return std::vector(ids.begin(), ids.end());
}

ActionId ActionId::generate() {
    static std::unique_ptr<ActionId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<ActionId::Generator>(getSeed());

    return (*generator)();
}

TournamentId TournamentId::generate() {
    static std::unique_ptr<TournamentId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<TournamentId::Generator>(getSeed());

    return (*generator)();
}

ClientId ClientId::generate() {
    static std::unique_ptr<ClientId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<ClientId::Generator>(getSeed());

    return (*generator)();
}

ClientActionId::ClientActionId() {}

ClientActionId::ClientActionId(const ClientId &clientId, const ActionId &actionId)
    : mClientId(clientId)
    , mActionId(actionId)
{}

bool ClientActionId::operator==(const ClientActionId &other) const {
    return mClientId == other.mClientId && mActionId == other.mActionId;
}

bool ClientActionId::operator!=(const ClientActionId &other) const {
    return mClientId != other.mClientId || mActionId != other.mActionId;
}

bool ClientActionId::operator<(const ClientActionId &other) const {
    if (mClientId != other.mClientId)
        return mClientId < other.mClientId;
    return mActionId < other.mActionId;
}

ClientId ClientActionId::getClientId() const {
    return mClientId;
}

ActionId ClientActionId::getActionId() const {
    return mActionId;
}

std::ostream & operator<<(std::ostream & o, ClientActionId id) {
    return o << std::hex << "(" << id.getClientId().getValue() << "; " << id.getActionId().getValue() << ")" << std::dec;
}

CombinedId::CombinedId() {

}

CombinedId::CombinedId(CategoryId categoryId, MatchId matchId)
    : mCategoryId(categoryId)
    , mMatchId(matchId)
{
}

bool CombinedId::operator==(const CombinedId &other) const {
    return mCategoryId == other.mCategoryId && mMatchId == other.mMatchId;
}

bool CombinedId::operator!=(const CombinedId &other) const {
    return mCategoryId != other.mCategoryId || mMatchId != other.mMatchId;
}

bool CombinedId::operator<(const CombinedId &other) const {
    if (mCategoryId != other.mCategoryId)
        return mCategoryId < other.mCategoryId;
    return mMatchId < other.mMatchId;
}

std::string CombinedId::toString() const {
    return mCategoryId.toString() + ";" + mMatchId.toString();
}

