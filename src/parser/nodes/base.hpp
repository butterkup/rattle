#pragma once

namespace rat::parser::nodes {
#define ExprTitles(_) \
  _(Plus) _(Minus) _(Star)_(Percent) \
  _(Slash) _(Less) _(BitAnd) _(Invert) \
  _(Greater) _(Lshift) _(Rshift) _(BitOr) \
  _(Group) _(Identifier) _(String) _(Dot) \
  _(Number) _(UnaryPlus) _(UnaryMinus) \
  _(UnaryAt) _(At) _(In) _(Not) _(And) \
  _(Or) _(True) _(False) _(Is) _(NotEqual)\
  _(IsNot) _(NotIn) _(EqualEqual)

#define StmtTitles(_) \
  _(Raise) _(Class) _(GreaterEqual) _(Import)  \
   _(Try) _(Else)  _(Global) _(SlashEqual) \
  _(While) _(For) _(Lastly) _(Except) \
  _(Return) _(With) _(Assert) _(From) \
  _(NonLocal)  _(Block) _(Yield) _(RshiftEqual) \
  _(Equal) _(PlusEqual) _(LshiftEqual) _(If) \
  _(MinusEqual) _(StarEqual) _(PercentEqual)  \
  _(LessEqual) _(Fn)  _(InvertEqual) _(Del)  \
  _(BitOrEqual) _(BitAndEqual) _(AtEqual) \

  struct Visitor;

  struct Node {
    virtual void accept(Visitor &) = 0;
  };

  struct Expr: Node { };

#define _F(Name) struct Name;
  ExprTitles(_F);
  StmtTitles(_F);
#undef _F

#define _F(Klass) virtual void accept(Klass&) = 0;
  struct ExprVisitor {
    ExprTitles(_F);
  };

  struct Visitor: ExprVisitor {
    StmtTitles(_F);
  };

#undef _F
}
