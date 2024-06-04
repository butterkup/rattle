#ifndef _rat_config
#define _rat_config

#include <string>
#include <string_view>
#include <vector>
#include <queue>

namespace rat::core {
  struct Source {
    const std::string source;
    std::vector<const std::string_view> lines;
  };

  struct SyntaxError {
    std::size_t
      line,
      column,
      offset,
      span;
  };

  struct Config {
    Source source;
    std::queue<const SyntaxError> errors;
  };
}

#endif //_rat_config
