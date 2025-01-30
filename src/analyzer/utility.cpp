#include <rattle/analyzer.hpp>

namespace rattle {
  analyzer::ContextSettings Analyzer::with(Analyzer::Context val) {
    return {*this, val};
  }
  auto Analyzer::test(Analyzer::Context val) const {
    return settings.back() & val;
  }
} // namespace rattle

