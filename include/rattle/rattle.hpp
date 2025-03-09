#ifndef rattle_config_h
#define rattle_config_h

#include <cassert>
#include <memory>
#include <type_traits>

namespace rattle {
  namespace internal {
    // Fallback implementation as std::unreachable was
    // added in c++23 and so far we are targetting c++20
    [[noreturn]]
    inline void unreachable() noexcept {
      assert(false && "The unreachable has been reached!");
#if defined(__MSC_VER)
      __assume(false);
#elif defined(__GNUC__) || defined(__clang__)
      __builtin_unreachable();
#else
# error "What compiler are you using?"
#endif
    }
  } // namespace internal
  using internal::unreachable;

  // Safely take the base as a child as per API, if an
  // error creeps in, we should be able to find it unlike
  // casting pointers which prove unreliable.
  template <typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  inline Derived &rattle_cast(Base &base) noexcept {
    if (Derived *child = dynamic_cast<Derived *>(std::addressof(base))) {
      return *child;
    }
    unreachable();
  }
} // namespace rattle

#endif

