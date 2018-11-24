#include <chrono>

#include "id.hpp"
#include "draw_systems/draw_system.hpp"
#include "rulesets/ruleset.hpp"
#include "stores/category_store.hpp"
#include "stores/match_event.hpp"
#include "stores/tournament_store.hpp"

unsigned int getGeneratorSeed() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}

MatchId MatchId::generate(const CategoryStore &category) {
    static std::unique_ptr<MatchId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<MatchId::Generator>(getGeneratorSeed());

    MatchId id;
    do {
        id = (*generator)();
    } while(category.containsMatch(id));

    return id;
}

PlayerId PlayerId::generate(const TournamentStore &tournament) {
    static std::unique_ptr<PlayerId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<PlayerId::Generator>(getGeneratorSeed());

    PlayerId id;
    do {
        id = (*generator)();
    } while(tournament.containsPlayer(id));

    return id;
}

CategoryId CategoryId::generate(const TournamentStore &tournament) {
    static std::unique_ptr<CategoryId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<CategoryId::Generator>(getGeneratorSeed());

    CategoryId id;
    do {
        id = (*generator)();
    } while(tournament.containsCategory(id));

    return id;
}

ActionId ActionId::generate() {
    static std::unique_ptr<ActionId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<ActionId::Generator>(getGeneratorSeed());

    return (*generator)();
}

TournamentId TournamentId::generate() {
    static std::unique_ptr<TournamentId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<TournamentId::Generator>(getGeneratorSeed());

    return (*generator)();
}

ClientId ClientId::generate() {
    static std::unique_ptr<ClientId::Generator> generator; // singleton
    if (!generator)
        generator = std::make_unique<ClientId::Generator>(getGeneratorSeed());

    return (*generator)();
}
