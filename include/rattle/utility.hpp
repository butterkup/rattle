#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <type_traits>

namespace rattle::utility {
  // Very useful for printing strings with control characters.
  // Simply a type tag over which std::string_view printer to use:
  // ours for escaping special characters (control, etc); the one provided
  // by standard library.
  struct escape {
    std::string_view view;
  };

  // Stream the view into the ostream while escaping as needed
  std::ostream &operator<<(std::ostream &, escape) noexcept;

  // Get the string escaped as std::string
  std::string to_string(escape) noexcept;
  // What happens after an error?
  // A reactor can choose of the two techniques.
  enum class OnError {
    // Stop execution and cleanup.
    Abort,
    // Continue as if nothing happend
    Resume
  };
  // Call destructor of held pointer for cleanup.
  // Very lightweight wrapper, in fact, it is the same exact size
  // as a pointer type. Adds no overhead.
  template <class Type> class Scoped {
    static_assert(
      not std::is_reference_v<Type>, "`Type` cannot be a reference type.");
    static_assert(
      not std::is_volatile_v<Type>, "`Type` cannot be volatile qualified.");
    static_assert(
      not std::is_pointer_v<Type>, "`Type` cannot be a pointer type.");
    static_assert(
      not std::is_const_v<Type>, "`Type` cannot be const qualified.");
    static_assert(not std::is_array_v<Type>, "`Type` cannot be an array type.");
    static_assert(not std::is_void_v<Type>, "`Type` cannot be a void type.");
    Type *ptr;

  public:
    constexpr Scoped(Type *ptr = nullptr) noexcept: ptr{ptr} {}
    // Use after move is alowed but know that the moved from object will be non-owning.
    constexpr Scoped(Scoped &&w) noexcept: ptr{w.release()} {}
    // Allow polymorphic assignments; for safety, ensure virtual destructors
    template <class Derived>
      requires(std::is_base_of_v<Type, Derived> and
               std::has_virtual_destructor_v<Type> and
               not std::is_same_v<Type, Derived>)
    constexpr Scoped(Scoped<Derived> &&w) noexcept: ptr{w.release()} {}
    Scoped &operator=(Scoped &&w) noexcept(
      std::is_nothrow_destructible_v<Type>) {
      // Destroy whatver is held
      destroy();
      // Steal the resource from w.
      ptr = w.release();
      return *this;
    }
    // To prevent double destruction, no copying is allowed.
    constexpr Scoped(Scoped const &) = delete;
    Scoped &operator=(Scoped const &) = delete;
    // Check if Dtor is owning; managed ptr is not nullptr.
    bool is_owning() const noexcept { return ptr != nullptr; }
    operator bool() const noexcept { return is_owning(); }
    // Get internal pointer
    Type *get() const { return ptr; }
    // Release ownership of the pointer
    [[nodiscard]] Type *release() {
      Type *temp = ptr;
      ptr = nullptr;
      return temp;
    }
    // Ensure is_owning before dereference using either * or ->
    Type *operator->() const noexcept {
      assert(is_owning()); // useful for debugging
      return ptr;
    }
    Type &operator*() const noexcept {
      assert(is_owning()); // useful for debugging
      return *ptr;
    }
    // If owning, call the destructor and disown the pointer to prevent double destruction.
    void destroy() noexcept(std::is_nothrow_destructible_v<Type>) {
      if (is_owning()) {
        ptr->~Type();
        ptr = nullptr;
      }
    }
    // Destroy on destrution
    ~Scoped() noexcept(std::is_nothrow_destructible_v<Type>) { destroy(); }
  };

  // For lexing and beyond.
  bool is_decimal(int) noexcept;
  bool is_hexadecimal(int) noexcept;
  bool is_binary(int) noexcept;
  bool is_octal(int) noexcept;
  bool is_identifier_start_char(int) noexcept;
  bool is_identifier_body_char(int) noexcept;
  bool is_whitespace(int) noexcept;
} // namespace rattle::utility
