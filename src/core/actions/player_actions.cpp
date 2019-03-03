#include "core/actions/player_actions.hpp"
#include "core/actions/category_actions.hpp"
#include "core/actions/match_actions.hpp"
#include "core/draw_systems/draw_system.hpp"
#include "core/exception.hpp"
#include "core/random.hpp"
#include "core/rulesets/ruleset.hpp"
#include "core/stores/category_store.hpp"
#include "core/stores/tournament_store.hpp"

AddPlayersAction::AddPlayersAction(TournamentStore & tournament, const std::vector<PlayerFields> &fields)
    : mFields(fields)
{
    for (size_t i = 0; i < fields.size(); ++i)
        mIds.push_back(PlayerId::generate(tournament));
}

AddPlayersAction::AddPlayersAction(const std::vector<PlayerId> &ids, const std::vector<PlayerFields> &fields)
    : mIds(ids)
    , mFields(fields)
{
    assert(mIds.size() == mFields.size());
}

void AddPlayersAction::redoImpl(TournamentStore & tournament) {
    for (auto id : mIds) {
        if (tournament.containsPlayer(id))
            throw ActionExecutionException("Failed to redo AddPlayersAction. Player already exists."); // TODO: Make sure to catch these exceptions
    }

    tournament.beginAddPlayers(mIds);
    auto i = mIds.begin();
    auto j = mFields.begin();
    for (; i != mIds.end() && j != mFields.end(); ++i, ++j)
        tournament.addPlayer(std::make_unique<PlayerStore>(*i, *j));

    tournament.endAddPlayers();
}

void AddPlayersAction::undoImpl(TournamentStore & tournament) {
    tournament.beginErasePlayers(mIds);
    for (auto id : mIds)
        tournament.erasePlayer(id);
    tournament.endErasePlayers();
}

ErasePlayersAction::ErasePlayersAction(const std::vector<PlayerId> &playerIds)
    : mPlayerIds(playerIds)
    , mSeed(getSeed())
{}

ErasePlayersAction::ErasePlayersAction(const std::vector<PlayerId> &playerIds, unsigned int seed)
    : mPlayerIds(playerIds)
    , mSeed(seed)
{}

std::unique_ptr<Action> ErasePlayersAction::freshClone() const {
    return std::make_unique<ErasePlayersAction>(mPlayerIds, mSeed);
}

void ErasePlayersAction::redoImpl(TournamentStore & tournament) {
    std::unordered_set<CategoryId> categoryIds;

    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId)) continue;

        mErasedPlayerIds.push_back(playerId);

        const PlayerStore &player = tournament.getPlayer(playerId);
        categoryIds.insert(player.getCategories().begin(), player.getCategories().end());
    }

    tournament.beginErasePlayers(mErasedPlayerIds);
    for (auto categoryId : categoryIds) {
        auto action = std::make_unique<ErasePlayersFromCategoryAction>(categoryId, mErasedPlayerIds, mSeed); // lazily give the action all playerIds and let it figure the rest out on its own
        action->redo(tournament);
        mActions.push(std::move(action));
    }

    for (auto playerId : mErasedPlayerIds)
        mPlayers.push(std::move(tournament.erasePlayer(playerId)));
    tournament.endErasePlayers();
}

void ErasePlayersAction::undoImpl(TournamentStore & tournament) {
    tournament.beginAddPlayers(mErasedPlayerIds);
    while (!mPlayers.empty()) {
        tournament.addPlayer(std::move(mPlayers.top()));
        mPlayers.pop();
    }
    tournament.endAddPlayers();
    mErasedPlayerIds.clear();

    while (!mActions.empty()) {
        mActions.top()->undo(tournament);
        mActions.pop();
    }
}

ChangePlayersFirstNameAction::ChangePlayersFirstNameAction(std::vector<PlayerId> playerIds, const std::string &value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

std::unique_ptr<Action> ChangePlayersFirstNameAction::freshClone() const {
    return std::make_unique<ChangePlayersFirstNameAction>(mPlayerIds, mValue);
}

void ChangePlayersFirstNameAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getFirstName());
        player.setFirstName(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersFirstNameAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setFirstName(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

ChangePlayersLastNameAction::ChangePlayersLastNameAction(std::vector<PlayerId> playerIds, const std::string &value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersLastNameAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getLastName());
        player.setLastName(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersLastNameAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setLastName(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

ChangePlayersClubAction::ChangePlayersClubAction(std::vector<PlayerId> playerIds, const std::string &value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersClubAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getClub());
        player.setClub(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersClubAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setClub(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

ChangePlayersAgeAction::ChangePlayersAgeAction(std::vector<PlayerId> playerIds, std::optional<PlayerAge> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersAgeAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getAge());
        player.setAge(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersAgeAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setAge(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

ChangePlayersRankAction::ChangePlayersRankAction(std::vector<PlayerId> playerIds, std::optional<PlayerRank> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersRankAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getRank());
        player.setRank(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersRankAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setRank(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

ChangePlayersWeightAction::ChangePlayersWeightAction(std::vector<PlayerId> playerIds, std::optional<PlayerWeight> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersWeightAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getWeight());
        player.setWeight(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersWeightAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setWeight(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

ChangePlayersCountryAction::ChangePlayersCountryAction(std::vector<PlayerId> playerIds, std::optional<PlayerCountry> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersCountryAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getCountry());
        player.setCountry(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersCountryAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setCountry(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

ChangePlayersSexAction::ChangePlayersSexAction(std::vector<PlayerId> playerIds, std::optional<PlayerSex> value)
    : mPlayerIds(playerIds)
    , mValue(value)
{}

void ChangePlayersSexAction::redoImpl(TournamentStore & tournament) {
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;

        PlayerStore & player = tournament.getPlayer(playerId);
        mOldValues.push_back(player.getSex());
        player.setSex(mValue);
    }
    tournament.changePlayers(mPlayerIds);
}

void ChangePlayersSexAction::undoImpl(TournamentStore & tournament) {
    auto i = mOldValues.begin();
    for (auto playerId : mPlayerIds) {
        if (!tournament.containsPlayer(playerId))
            continue;
        assert(i != mOldValues.end());

        PlayerStore & player = tournament.getPlayer(playerId);
        player.setSex(*i);

        std::advance(i, 1);
    }

    tournament.changePlayers(mPlayerIds);
    mOldValues.clear();
}

std::unique_ptr<Action> AddPlayersAction::freshClone() const {
    return std::make_unique<AddPlayersAction>(mIds, mFields);
}

std::unique_ptr<Action> ChangePlayersLastNameAction::freshClone() const {
    return std::make_unique<ChangePlayersLastNameAction>(mPlayerIds, mValue);
}

std::unique_ptr<Action> ChangePlayersAgeAction::freshClone() const {
    return std::make_unique<ChangePlayersAgeAction>(mPlayerIds, mValue);
}

std::unique_ptr<Action> ChangePlayersClubAction::freshClone() const {
    return std::make_unique<ChangePlayersClubAction>(mPlayerIds, mValue);
}

std::unique_ptr<Action> ChangePlayersRankAction::freshClone() const {
    return std::make_unique<ChangePlayersRankAction>(mPlayerIds, mValue);
}

std::unique_ptr<Action> ChangePlayersSexAction::freshClone() const {
    return std::make_unique<ChangePlayersSexAction>(mPlayerIds, mValue);
}

std::unique_ptr<Action> ChangePlayersWeightAction::freshClone() const {
    return std::make_unique<ChangePlayersWeightAction>(mPlayerIds, mValue);
}

std::unique_ptr<Action> ChangePlayersCountryAction::freshClone() const {
    return std::make_unique<ChangePlayersCountryAction>(mPlayerIds, mValue);
}

std::string AddPlayersAction::getDescription() const {
    return "Add players";
}

std::string ErasePlayersAction::getDescription() const {
    return "Erase players";
}

std::string ChangePlayersFirstNameAction::getDescription() const {
    return "Change players first name";
}

std::string ChangePlayersLastNameAction::getDescription() const {
    return "Change players last name";
}

std::string ChangePlayersAgeAction::getDescription() const {
    return "Change players age";
}

std::string ChangePlayersRankAction::getDescription() const {
    return "Change players rank";
}

std::string ChangePlayersClubAction::getDescription() const {
    return "Change players club";
}

std::string ChangePlayersWeightAction::getDescription() const {
    return "Change players weight";
}

std::string ChangePlayersCountryAction::getDescription() const {
    return "Change players country";
}

std::string ChangePlayersSexAction::getDescription() const {
    return "Change players sex";
}

