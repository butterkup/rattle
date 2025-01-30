#pragma once

#include <deque>
#ifndef RATTLE_SOURCE_ONLY
#include "category.hpp"
#include "lexer.hpp"
#include "parser_nodes.hpp"
#endif

namespace rattle {
  namespace analyzer {
    enum class node_t {
#define TK_MACRO(Name, _) Name,
#define TK_INCLUDE TK_ALL_NODES
#include "token_macro.hpp"
    };

    const char *to_string(node_t type);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#define CREATE_VISIT(_Type) void visit(parser::nodes::_Type &node) override;

    struct NodeType: parser::nodes::Visitor {
      NodeType(): type(node_t::Statement) {}
      node_t get_type(parser::nodes::Statement &node);

#define TK_INCLUDE TK_ALL_NODES
#define TK_MACRO(Name, _) CREATE_VISIT(Name)
#include <rattle/token_macro.hpp>
    private:
      node_t type;
    };

    enum class error_t {
#define ERROR_MACRO(error) error,
#define ERROR_INCLUDE ANALYZER_ERROR
#include "error_macro.hpp"
    };

    struct Error {
      error_t error;
      rattle::lexer::Location start, end;
      Error(error_t error, lexer::Location const &start,
            lexer::Location const &end)
        : error(error), start(start), end(end) {}
    };

    struct ContextSettings;
  } // namespace analyzer

  struct Analyzer: parser::nodes::Visitor {
#define TK_INCLUDE TK_ALL_NODES
#define TK_MACRO(Name, _) CREATE_VISIT(Name)
#include <rattle/token_macro.hpp>

    enum Context {
      NONE,
      PARAMETERS,
      LOOP,
      FUNCTION,
      CLASS,
      TUPLE,
      LIST,
    };

    auto test(Context val) const;
    analyzer::ContextSettings with(Context val = NONE);
    void report(analyzer::error_t error, lexer::Location const &start,
                lexer::Location const &end);
    void report(analyzer::error_t error, lexer::Token const &start,
                lexer::Token const &end);
    void report(analyzer::error_t error, lexer::Token const &token);

  private:
    friend struct analyzer::ContextSettings;

    void check_binary_ops(parser::nodes::BinaryExpr &expr);

    std::deque<Context> settings;
    std::deque<analyzer::Error> errors;
  };

  struct analyzer::ContextSettings {
    Analyzer &analyzer;
    ContextSettings(Analyzer &analyzer): analyzer(analyzer) {}
    ContextSettings(Analyzer &analyzer, Analyzer::Context value)
      : analyzer(analyzer) {
      analyzer.settings.push_back(value);
    }
    ~ContextSettings() { analyzer.settings.pop_back(); }
  };

#undef CREATE_VISIT

#pragma GCC diagnostic pop
} // namespace rattle

