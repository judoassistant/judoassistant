#pragma once

#include <vector>

#include "core/core.hpp"
#include "core/id.hpp"
#include "core/serialize.hpp"
#include "core/draw_systems/draw_system_identifier.hpp"

class Action;
class AddMatchAction;
class CategoryStore;
class TournamentStore;

class DrawSystem {
public:
    typedef std::vector<std::pair<PlayerId, std::optional<unsigned int>>> ResultList;

    virtual ~DrawSystem() {};

    virtual std::unique_ptr<DrawSystem> clone() const = 0;
    virtual std::string getName() const = 0;
    virtual bool hasFinalBlock() const = 0;
    virtual DrawSystemIdentifier getIdentifier() const = 0; // Used for comparing draw systems

    virtual std::vector<std::unique_ptr<AddMatchAction>> initCategory(const TournamentStore &tournament, const CategoryStore &category, const std::vector<PlayerId> &playerIds, unsigned int seed) = 0;
    virtual std::vector<std::unique_ptr<Action>> updateCategory(const TournamentStore &tournament, const CategoryStore &category) const = 0;
    virtual ResultList getResults(const TournamentStore &tournament, const CategoryStore &category) const = 0;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {}

    static const std::vector<std::unique_ptr<const DrawSystem>> & getDrawSystems();
    static std::unique_ptr<DrawSystem> getDrawSystem(DrawSystemIdentifier identifier);
    static std::size_t getDrawSystemIndex(DrawSystemIdentifier identifier);
};

