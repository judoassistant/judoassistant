#include <chrono>

#include "id.hpp"
#include "hash.hpp"
#include "draw_systems/draw_system.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/category_store.hpp"
#include "stores/tournament_store.hpp"

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

