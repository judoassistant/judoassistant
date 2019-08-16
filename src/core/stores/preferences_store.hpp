#pragma once

#include "core/serialize.hpp"
#include "core/draw_systems/draw_system_identifier.hpp"

class DrawSystem;

struct DrawSystemPreference {
    std::size_t playerLowerLimit;
    DrawSystemIdentifier drawSystem;

    DrawSystemPreference() = default;
    DrawSystemPreference(std::size_t playerLowerLimit, DrawSystemIdentifier drawSystem);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(playerLowerLimit, drawSystem);
    }
};

enum class ScoreboardStylePreference { NATIONAL, INTERNATIONAL };
enum class MatchCardStylePreference { NATIONAL };

class PreferencesStore {
public:
    PreferencesStore();

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPreferredDrawSystems, mScoreboardStyle, mMatchCardStyle);
    }

    const std::vector<DrawSystemPreference>& getPreferredDrawSystems() const;
    std::vector<DrawSystemPreference>& getPreferredDrawSystems();

    DrawSystemIdentifier getPreferredDrawSystem(std::size_t size) const;

    ScoreboardStylePreference getScoreboardStyle() const;
    void setScoreboardStyle(ScoreboardStylePreference style);

    MatchCardStylePreference getMatchCardStyle() const;
    void setMatchCardStyle(MatchCardStylePreference style);

private:
    std::vector<DrawSystemPreference> mPreferredDrawSystems;
    ScoreboardStylePreference mScoreboardStyle;
    MatchCardStylePreference mMatchCardStyle;
};

