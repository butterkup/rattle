#include <rattle/analyzer.hpp>
#include <rattle/parser_nodes.hpp>

namespace rattle {
  using namespace parser::nodes;
  void Analyzer::check_binary_ops(BinaryExpr &expr) {
    if (not expr.left_operand) {
      report(analyzer::error_t::missing_left_operand, expr.token);
    }
    if (not expr.right_operand) {
      report(analyzer::error_t::missing_right_operand, expr.token);
    }
  }

  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Minus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }
  void Analyzer::visit(Plus &expr) { check_binary_ops(expr); }

} // namespace rattle

