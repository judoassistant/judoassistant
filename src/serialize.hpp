#pragma once

#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/polymorphic.hpp>

namespace cereal {
  template <class Archive, typename T> inline
  void CEREAL_SAVE_FUNCTION_NAME(Archive& ar, const std::optional<T>& optional)
  {
    if(!optional.has_value()) {
      ar(CEREAL_NVP_("nullopt", true));
    } else {
      ar(CEREAL_NVP_("nullopt", false),
         CEREAL_NVP_("value", *optional));
    }
  }

  template <class Archive, typename T> inline
  void CEREAL_LOAD_FUNCTION_NAME(Archive& ar, std::optional<T>& optional)
  {
    bool nullopt;
    ar(CEREAL_NVP_("nullopt", nullopt));

    if (nullopt) {
      optional = std::nullopt;
    } else {
      T value;
      ar(CEREAL_NVP_("value", value));
      optional = std::move(value);
    }
  }
} // namespace cereal

