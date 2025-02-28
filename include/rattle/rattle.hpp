#ifndef rattle_config_h
#define rattle_config_h

#include <cassert>

namespace rattle {
  namespace internal {
    // Fallback implementation as std::unreachable was
    // added in c++23 and so far we are targetting c++20
    [[noreturn]]
    inline void unreachable() noexcept {
#ifndef NDEBUG
      assert(false && "Unreachable has been reached!");
#elif defined(__MSC_VER)
      __assume(false);
#elif defined(__GNUC__) || defined(__clang__)
      __builtin_unreachable();
#else
# error "What compiler are you using?"
#endif
    }
  } // namespace internal
  using internal::unreachable;
} // namespace rattle

#endif

