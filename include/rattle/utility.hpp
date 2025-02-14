#pragma once

#include <string>
#include <string_view>

namespace rattle::utility {
  // Very useful for printing strings with control characters.
  // Simply a type tag over which std::string_view printer to use:
  // ours for escaping special characters (control, etc); the one provided
  // by standard library.
  struct escape {
    std::string_view view;
  };

  // Stream the view into the ostream while escaping as needed
  std::ostream &operator<<(std::ostream &, escape);

  // Get the string escaped as std::string
  std::string to_string(escape);
  // What happens after an error?
  // A reactor can choose of the two techniques.
  enum class OnError {
    // Stop execution and cleanup.
    Abort,
    // Continue as if nothing happend
    Resume
  };
} // namespace rattle::utility
