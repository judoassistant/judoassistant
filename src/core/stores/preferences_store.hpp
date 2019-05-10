#pragma once

#include "core/serialize.hpp"

class DrawSystem;

struct DrawSystemPreference {
    std::size_t playerLowerLimit;
    std::unique_ptr<const DrawSystem> drawSystem;

    DrawSystemPreference() = default;
    DrawSystemPreference(std::size_t playerLowerLimit, std::unique_ptr<DrawSystem> drawSystem);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(playerLowerLimit, drawSystem);
    }
};

class PreferencesStore {
public:
    PreferencesStore();
    PreferencesStore(const PreferencesStore &other);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPreferredDrawSystems);
    }

    const std::vector<DrawSystemPreference>& getPreferredDrawSystems() const;
    std::vector<DrawSystemPreference>& getPreferredDrawSystems();

    const std::unique_ptr<const DrawSystem>& getPreferredDrawSystem(std::size_t size) const;

private:
    std::vector<std::pair<std::size_t, std::unique_ptr<DrawSystem>>> mPreferredDrawSystems;
};

