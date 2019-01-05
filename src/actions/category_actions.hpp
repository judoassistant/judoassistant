#pragma once

#include "core.hpp"
#include "id.hpp"
#include "serialize.hpp"
#include "actions/action.hpp"

class CategoryStore;
class DrawSystem;
class TournamentStore;
class BlockLocation;
class MatchStore;
enum class MatchType;
class Ruleset;

class AddCategoryAction : public Action {
public:
    AddCategoryAction() = default;
    AddCategoryAction(TournamentStore & tournament, const std::string &name, uint8_t ruleset, uint8_t drawSystem);
    AddCategoryAction(CategoryId id, const std::string &name, uint8_t ruleset, uint8_t drawSystem);

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
    uint8_t mRuleset;
    uint8_t mDrawSystem;
};

CEREAL_REGISTER_TYPE(AddCategoryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AddCategoryAction)

class DrawCategoryAction : public Action {
public:
    DrawCategoryAction() = default;
    DrawCategoryAction(CategoryId categoryId);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId);
    }

private:
    CategoryId mCategoryId;

    // undo members
    std::vector<std::unique_ptr<MatchStore>> mOldMatches;
    std::stack<std::unique_ptr<Action>> mActions;
    std::unique_ptr<DrawSystem> mOldDrawSystem;
};

CEREAL_REGISTER_TYPE(DrawCategoryAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, DrawCategoryAction)

class AddPlayersToCategoryAction : public Action {
public:
    AddPlayersToCategoryAction() = default;
    AddPlayersToCategoryAction(CategoryId categoryId, const std::vector<PlayerId> &playerIds);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId);
        ar(mPlayerIds);
    }

private:
    CategoryId mCategoryId;
    std::vector<PlayerId> mPlayerIds;

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
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId);
        ar(mPlayerIds);
    }

private:
    CategoryId mCategoryId;
    std::vector<PlayerId> mPlayerIds;

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
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
    }

private:
    std::vector<PlayerId> mPlayerIds;

    // undo members
    std::stack<std::unique_ptr<ErasePlayersFromCategoryAction>> mActions;
};

CEREAL_REGISTER_TYPE(ErasePlayersFromAllCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ErasePlayersFromAllCategoriesAction)

class AutoAddCategoriesAction : public Action {
public:
    AutoAddCategoriesAction() = default;
    AutoAddCategoriesAction(TournamentStore &tournament, std::vector<PlayerId> playerIds, std::string baseName, float maxDifference, size_t maxSize);
    AutoAddCategoriesAction(const std::vector<std::vector<PlayerId>> &playerIds, std::vector<CategoryId> categoryIds, std::string baseName);

    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPlayerIds);
        ar(mCategoryIds);
        ar(mBaseName);
    }

private:
    std::vector<std::vector<PlayerId>> mPlayerIds;
    std::vector<CategoryId> mCategoryIds;
    std::string mBaseName;
};

CEREAL_REGISTER_TYPE(AutoAddCategoriesAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, AutoAddCategoriesAction)

class ChangeCategoryNameAction : public Action {
public:
    ChangeCategoryNameAction() = default;
    ChangeCategoryNameAction(CategoryId categoryId, const std::string &value);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId);
        ar(mValue);
    }

private:
    CategoryId mCategoryId;
    std::string mValue;

    // undo members
    std::string mOldValue;
};

CEREAL_REGISTER_TYPE(ChangeCategoryNameAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoryNameAction)

class ChangeCategoryRulesetAction : public Action {
public:
    ChangeCategoryRulesetAction() = default;
    ChangeCategoryRulesetAction (CategoryId categoryId, uint8_t ruleset);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId);
        ar(mRuleset);
    }

private:
    CategoryId mCategoryId;
    uint8_t mRuleset;

    // undo members
    std::unique_ptr<Ruleset> mOldRuleset;
    std::unique_ptr<DrawCategoryAction> mDrawAction;
};

CEREAL_REGISTER_TYPE(ChangeCategoryRulesetAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoryRulesetAction)

class ChangeCategoryDrawSystemAction : public Action {
public:
    ChangeCategoryDrawSystemAction() = default;
    ChangeCategoryDrawSystemAction(CategoryId categoryId, uint8_t drawSystem);
    void redoImpl(TournamentStore & tournament) override;
    void undoImpl(TournamentStore & tournament) override;

    std::unique_ptr<Action> freshClone() const override;
    std::string getDescription() const override;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId);
        ar(mDrawSystem);
    }

private:
    CategoryId mCategoryId;
    uint8_t mDrawSystem;

    // undo members
    std::unique_ptr<DrawSystem> mOldDrawSystem;
    std::unique_ptr<DrawCategoryAction> mDrawAction;
};

CEREAL_REGISTER_TYPE(ChangeCategoryDrawSystemAction)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Action, ChangeCategoryDrawSystemAction)

