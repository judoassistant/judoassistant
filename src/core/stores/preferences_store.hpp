#pragma once

#include "core/serialize.hpp"

class DrawSystem;

class PreferencesStore {
public:
    PreferencesStore();
    PreferencesStore(const PreferencesStore &other);

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mPreferredDrawSystems);
    }

private:
    std::vector<std::pair<std::size_t, std::unique_ptr<DrawSystem>>> mPreferredDrawSystems;
};

