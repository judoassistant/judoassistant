#include "actions/player_actions.hpp"
#include "actions/category_actions.hpp"
#include "actions/match_actions.hpp"
#include "stores/tournament_store.hpp"
#include "exception.hpp"

AddPlayerAction::AddPlayerAction(TournamentStore & tournament, const std::string & firstName, const std::string & lastName, std::optional<PlayerAge> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<PlayerWeight> weight, std::optional<PlayerCountry> country, std::optional<PlayerSex> sex)
    : AddPlayerAction(PlayerId::generate(tournament), firstName, lastName, age, rank, club, weight, country, sex)
{}

AddPlayerAction::AddPlayerAction(PlayerId id, const std::string & firstName, const std::string & lastName, std::optional<PlayerAge> age, std::optional<PlayerRank> rank, const std::string &club, std::optional<PlayerWeight> weight, std::optional<PlayerCountry> country, std::optional<PlayerSex> sex)
    : mId(id)
    , mFirstName(firstName)
    , mLastName(lastName)
    , mAge(age)
    , mRank(rank)
    , mClub(club)
    , mWeight(weight)
    , mCountry(country)
    , mSex(sex)
{}

void AddPlayerAction::redoImpl(TournamentStore & tournament) {
    if (tournament.containsPlayer(mId))
        throw ActionExecutionException("Failed to redo AddPlayerAction. Player already exists.");

    tournament.beginAddPlayers({mId});
    tournament.addPlayer(std::make_unique<PlayerStore>(mId, mFirstName, mLastName, mAge, mRank, mClub, mWeight, mCountry, mSex));
    tournament.endAddPlayers();
}

void AddPlayerAction::undoImpl(TournamentStore & tournament) {
    tournament.beginErasePlayers({mId});
    tournament.erasePlayer(mId);
    tournament.endErasePlayers();
}

ErasePlayersAction::ErasePlayersAction(const std::vector<PlayerId> &playerIds)
    : mPlayerIds(playerIds) // TODO: Use std::move where appropriate
{}

std::unique_ptr<Action> ErasePlayersAction::freshClone() const {
    return std::make_unique<ErasePlayersAction>(mPlayerIds);
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
        auto action = std::make_unique<ErasePlayersFromCategoryAction>(categoryId, mErasedPlayerIds); // lazily give the action all playerIds and let it figure the rest out on its own
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

ChangePlayerFirstNameAction::ChangePlayerFirstNameAction(PlayerId playerId, const std::string &value)
    : mPlayerId(playerId)
    , mValue(value)
{}

std::unique_ptr<Action> ChangePlayerFirstNameAction::freshClone() const {
    return std::make_unique<ChangePlayerFirstNameAction>(mPlayerId, mValue);
}

void ChangePlayerFirstNameAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    mOldValue = player.getFirstName();
    player.setFirstName(mValue);
    tournament.changePlayers({mPlayerId});
}

void ChangePlayerFirstNameAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    player.setFirstName(mOldValue);
    tournament.changePlayers({mPlayerId});
}

ChangePlayerLastNameAction::ChangePlayerLastNameAction(PlayerId playerId, const std::string &value)
    : mPlayerId(playerId)
    , mValue(value)
{}

void ChangePlayerLastNameAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    mOldValue = player.getLastName();
    player.setLastName(mValue);
    tournament.changePlayers({mPlayerId});
}

void ChangePlayerLastNameAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    player.setLastName(mOldValue);
    tournament.changePlayers({mPlayerId});
}

ChangePlayerClubAction::ChangePlayerClubAction(PlayerId playerId, const std::string &value)
    : mPlayerId(playerId)
    , mValue(value)
{}

void ChangePlayerClubAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    mOldValue = player.getClub();
    player.setClub(mValue);
    tournament.changePlayers({mPlayerId});
}

void ChangePlayerClubAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    player.setClub(mOldValue);
    tournament.changePlayers({mPlayerId});
}

ChangePlayerAgeAction::ChangePlayerAgeAction(PlayerId playerId, std::optional<PlayerAge> value)
    : mPlayerId(playerId)
    , mValue(value)
{}

void ChangePlayerAgeAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    mOldValue = player.getAge();
    player.setAge(mValue);
    tournament.changePlayers({mPlayerId});
}

void ChangePlayerAgeAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    player.setAge(mOldValue);
    tournament.changePlayers({mPlayerId});
}

ChangePlayerRankAction::ChangePlayerRankAction(PlayerId playerId, std::optional<PlayerRank> value)
    : mPlayerId(playerId)
    , mValue(value)
{}

void ChangePlayerRankAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    mOldValue = player.getRank();
    player.setRank(mValue);
    tournament.changePlayers({mPlayerId});
}

void ChangePlayerRankAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    player.setRank(mOldValue);
    tournament.changePlayers({mPlayerId});
}

ChangePlayerWeightAction::ChangePlayerWeightAction(PlayerId playerId, std::optional<PlayerWeight> value)
    : mPlayerId(playerId)
    , mValue(value)
{}

void ChangePlayerWeightAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    mOldValue = player.getWeight();
    player.setWeight(mValue);
    tournament.changePlayers({mPlayerId});
}

void ChangePlayerWeightAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    player.setWeight(mOldValue);
    tournament.changePlayers({mPlayerId});
}

ChangePlayerCountryAction::ChangePlayerCountryAction(PlayerId playerId, std::optional<PlayerCountry> value)
    : mPlayerId(playerId)
    , mValue(value)
{}

void ChangePlayerCountryAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    mOldValue = player.getCountry();
    player.setCountry(mValue);
    tournament.changePlayers({mPlayerId});
}

void ChangePlayerCountryAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    player.setCountry(mOldValue);
    tournament.changePlayers({mPlayerId});
}

ChangePlayerSexAction::ChangePlayerSexAction(PlayerId playerId, std::optional<PlayerSex> value)
    : mPlayerId(playerId)
    , mValue(value)
{}

void ChangePlayerSexAction::redoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    mOldValue = player.getSex();
    player.setSex(mValue);
    tournament.changePlayers({mPlayerId});
}

void ChangePlayerSexAction::undoImpl(TournamentStore & tournament) {
    if (!tournament.containsPlayer(mPlayerId))
        return;

    PlayerStore & player = tournament.getPlayer(mPlayerId);
    player.setSex(mOldValue);
    tournament.changePlayers({mPlayerId});
}

std::unique_ptr<Action> AddPlayerAction::freshClone() const {
    return std::make_unique<AddPlayerAction>(mId, mFirstName, mLastName, mAge, mRank, mClub, mWeight, mCountry, mSex);
}

std::unique_ptr<Action> ChangePlayerLastNameAction::freshClone() const {
    return std::make_unique<ChangePlayerLastNameAction>(mPlayerId, mValue);
}

std::unique_ptr<Action> ChangePlayerAgeAction::freshClone() const {
    return std::make_unique<ChangePlayerAgeAction>(mPlayerId, mValue);
}

std::unique_ptr<Action> ChangePlayerClubAction::freshClone() const {
    return std::make_unique<ChangePlayerClubAction>(mPlayerId, mValue);
}

std::unique_ptr<Action> ChangePlayerRankAction::freshClone() const {
    return std::make_unique<ChangePlayerRankAction>(mPlayerId, mValue);
}

std::unique_ptr<Action> ChangePlayerSexAction::freshClone() const {
    return std::make_unique<ChangePlayerSexAction>(mPlayerId, mValue);
}

std::unique_ptr<Action> ChangePlayerWeightAction::freshClone() const {
    return std::make_unique<ChangePlayerWeightAction>(mPlayerId, mValue);
}

std::unique_ptr<Action> ChangePlayerCountryAction::freshClone() const {
    return std::make_unique<ChangePlayerCountryAction>(mPlayerId, mValue);
}

