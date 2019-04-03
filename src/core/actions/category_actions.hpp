#pragma once

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"
#include "core/actions/action.hpp"
#include "core/stores/category_store.hpp"

class BlockLocation;
class DrawSystem;
class MatchStore;
class Ruleset;
class TournamentStore;
enum class MatchType;

class AddCategoryAction : public Action {
public:
    AddCategoryAction() = default;
    AddCategoryAction(TournamentStore & tournament, const std::string &name, size_t ruleset, size_t drawSystem);
    AddCategoryAction(CategoryId id, const std::string &name, size_t ruleset, size_t drawSystem);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;
    CategoryId getId() const;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId);
        ar(mName);
        ar(mRuleset);
        ar(mDrawSystem);
    }

private:
    CategoryId mId;
    std::string mName;
    size_t mRuleset;
    size_t mDrawSystem;
};

CEREAL_REGISTER_TYPE(AddCategoryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddCategoryAction)

class DrawCategoryAction : public Action {
public:
    DrawCategoryAction() = default;
    DrawCategoryAction(CategoryId categoryId);
    DrawCategoryAction(CategoryId categoryId, unsigned int seed);
    // DrawCategoryAction(CategoryId categoryId);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mSeed);
    }

private:
    CategoryId mCategoryId;
    unsigned int mSeed;

    // undo members
    std::vector<std::unique_ptr<MatchStore>> mOldMatches;
    std::stack<std::unique_ptr<Action>> mActions;
    std::unique_ptr<DrawSystem> mOldDrawSystem;
    std::array<CategoryStatus, 2> mOldStatus;
};

CEREAL_REGISTER_TYPE(DrawCategoryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, DrawCategoryAction)

class AddPlayersToCategoryAction : public Action {
public:
    AddPlayersToCategoryAction() = default;
    AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds);
    AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mPlayerIds, mSeed);
    }

private:
    CategoryId mCategoryId;
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::vector<PlayerId> mAddedPlayerIds;
    std::unique_ptr<DrawCategoryAction> mDrawAction;
};

CEREAL_REGISTER_TYPE(AddPlayersToCategoryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddPlayersToCategoryAction)

class ErasePlayersFromCategoryAction : public Action {
public:
    ErasePlayersFromCategoryAction() = default;
    ErasePlayersFromCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds);
    ErasePlayersFromCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mPlayerIds, mSeed);
    }

private:
    CategoryId mCategoryId;
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::vector<PlayerId> mErasedPlayerIds;
    std::unique_ptr<DrawCategoryAction> mDrawAction;
};

CEREAL_REGISTER_TYPE(ErasePlayersFromCategoryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ErasePlayersFromCategoryAction)

class EraseCategoriesAction : public Action {
public:
    EraseCategoriesAction() = default;
    EraseCategoriesAction(const std::vector<CategoryId> &categoryIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds);
    }

private:
    std::vector<CategoryId> mCategoryIds;

    // undo members
    std::vector<CategoryId> mErasedCategoryIds;
    std::stack<std::unique_ptr<CategoryStore>> mCategories;
    std::vector<BlockLocation> mLocations;
    std::vector<std::pair<CategoryId, MatchType>> mBlocks;
};

CEREAL_REGISTER_TYPE(EraseCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, EraseCategoriesAction)

class ErasePlayersFromAllCategoriesAction : public Action {
public:
    ErasePlayersFromAllCategoriesAction() = default;
    ErasePlayersFromAllCategoriesAction(const std::vector<PlayerId> &playerIds);
    ErasePlayersFromAllCategoriesAction(const std::vector<PlayerId> &playerIds, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds, mSeed);
    }

private:
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};

CEREAL_REGISTER_TYPE(ErasePlayersFromAllCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ErasePlayersFromAllCategoriesAction)

class AutoAddCategoriesAction : public Action {
public:
    AutoAddCategoriesAction() = default;
    AutoAddCategoriesAction(TournamentStore &tournament, std::vector<PlayerId> playerIds, std::string baseName, float maxDifference, size_t maxSize);
    AutoAddCategoriesAction(const std::vector<std::vector<PlayerId>> &playerIds, std::vector<CategoryId> categoryIds, std::string baseName, unsigned int seed);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds, mCategoryIds, mBaseName, mSeed);
    }

private:
    std::vector<std::vector<PlayerId>> mPlayerIds;
    std::vector<CategoryId> mCategoryIds;
    std::string mBaseName;
    unsigned int mSeed;
};

CEREAL_REGISTER_TYPE(AutoAddCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AutoAddCategoriesAction)

class ChangeCategoriesNameAction : public Action {
public:
    ChangeCategoriesNameAction() = default;
    ChangeCategoriesNameAction(std::vector<CategoryId> categoryIds, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds);
        ar(mValue);
    }

private:
    std::vector<CategoryId> mCategoryIds;
    std::string mValue;

    // undo members
    std::vector<std::string> mOldValues;
};

CEREAL_REGISTER_TYPE(ChangeCategoriesNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoriesNameAction)

class ChangeCategoriesRulesetAction : public Action {
public:
    ChangeCategoriesRulesetAction() = default;
    ChangeCategoriesRulesetAction(std::vector<CategoryId> categoryIds, size_t ruleset);
    ChangeCategoriesRulesetAction(std::vector<CategoryId> categoryIds, size_t ruleset, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mRuleset, mSeed);
    }

private:
    std::vector<CategoryId> mCategoryIds;
    size_t mRuleset;
    unsigned int mSeed;

    // undo members
    std::vector<std::unique_ptr<Ruleset>> mOldRulesets;
    std::vector<std::unique_ptr<DrawCategoryAction>> mDrawActions;
};

CEREAL_REGISTER_TYPE(ChangeCategoriesRulesetAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoriesRulesetAction)

class ChangeCategoriesDrawSystemAction : public Action {
public:
    ChangeCategoriesDrawSystemAction() = default;
    ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, size_t drawSystem);
    ChangeCategoriesDrawSystemAction(std::vector<CategoryId> categoryIds, size_t drawSystem, unsigned int seed);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryIds, mDrawSystem, mSeed);
    }

private:
    std::vector<CategoryId> mCategoryIds;
    size_t mDrawSystem;
    unsigned int mSeed;

    // undo members
    std::vector<std::unique_ptr<DrawSystem>> mOldDrawSystems;
    std::vector<std::unique_ptr<DrawCategoryAction>> mDrawActions;
};

CEREAL_REGISTER_TYPE(ChangeCategoriesDrawSystemAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoriesDrawSystemAction)

class AddCategoryWithPlayersAction : public Action {
public:
    AddCategoryWithPlayersAction() = default;
    AddCategoryWithPlayersAction(TournamentStore & tournament, const std::string &name, size_t ruleset, size_t drawSystem, const std::vector<PlayerId> &playerIds);
    AddCategoryWithPlayersAction(CategoryId id, const std::string &name, size_t ruleset, size_t drawSystem, const std::vector<PlayerId> &playerIds, unsigned int seed);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mId, mName, mRuleset, mDrawSystem, mPlayerIds, mSeed);
    }

private:
    CategoryId mId;
    std::string mName;
    size_t mRuleset;
    size_t mDrawSystem;
    std::vector<PlayerId> mPlayerIds;
    unsigned int mSeed;

    // undo members
    std::unique_ptr<AddCategoryAction> mCategoryAction;
    std::unique_ptr<AddPlayersToCategoryAction> mPlayersAction;
};

CEREAL_REGISTER_TYPE(AddCategoryWithPlayersAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddCategoryWithPlayersAction)

