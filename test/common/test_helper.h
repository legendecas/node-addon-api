#pragma once
#include "napi.h"

namespace Napi {

// Use this when a variable or parameter is unused in order to explicitly
// silence a compiler warning about that.
template <typename T>
inline void USE(T&&) {}

/**
 * A test helper that converts MaybeOrValue<T> to T by checking that
 * MaybeOrValue is NOT an empty Maybe when NODE_ADDON_API_ENABLE_MAYBE is
 * defined.
 *
 * Do nothing when NODE_ADDON_API_ENABLE_MAYBE is not defined.
 */
template <typename T>
inline T MaybeToChecked(MaybeOrValue<T> maybe) {
#if defined(NODE_ADDON_API_ENABLE_MAYBE)
  return maybe.ToChecked();
#else
  return maybe;
#endif
}

/**
 * A test helper that converts MaybeOrValue<T> to T by getting the value that
 * wrapped by the Maybe or return the default_value if the Maybe is empty when
 * NODE_ADDON_API_ENABLE_MAYBE is defined.
 *
 * Do nothing when NODE_ADDON_API_ENABLE_MAYBE is not defined.
 */
template <typename T>
inline T FromMaybe(MaybeOrValue<T> maybe, const T& default_value = T()) {
#if defined(NODE_ADDON_API_ENABLE_MAYBE)
  return maybe.FromMaybe(default_value);
#else
  USE(default_value);
  return maybe;
#endif
}

}  // namespace Napi
