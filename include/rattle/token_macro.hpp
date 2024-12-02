// clang-format off
#ifndef TK_MACRO
# error "Must define macro TK_MACRO(kind, string_rep) to include this template."
# define TK_MACRO(_, __) // to stop clangd from complaining, unreachable
#endif

#include "category.hpp"

#ifndef TK_INCLUDE
# define TK_INCLUDE TK_ALL
#endif

#if TK_INCLUDE & TK_SPECIAL
 TK_MACRO(Error, "ErrorToken")   // Invalid characters and constructs
 TK_MACRO(Eot, "EndOfToken")     // Marks end of token stream
 TK_MACRO(Eos, "EndOfStatement") // Can be newline and semicolon
#endif

#if TK_INCLUDE & TK_KEYOTHER
 TK_MACRO(Raise, "raise")
 TK_MACRO(Class, "class")
 TK_MACRO(Fn, "fn")
 TK_MACRO(Import, "import")
 TK_MACRO(Try, "try")
 TK_MACRO(Else, "else")
 TK_MACRO(If, "if")
 TK_MACRO(While, "while")
 TK_MACRO(For, "for")
 TK_MACRO(Lastly, "lastly")
 TK_MACRO(Except, "except")
 TK_MACRO(Return, "return")
 TK_MACRO(With, "with")
 TK_MACRO(Assert, "assert")
 TK_MACRO(From, "from")
 TK_MACRO(NonLocal, "nonlocal")
 TK_MACRO(Global, "global")
 TK_MACRO(Del, "del")
 TK_MACRO(Continue, "continue")
 TK_MACRO(Break, "break")
#endif

#if TK_INCLUDE & TK_KEYUNARY
 TK_MACRO(Yield, "yield")
 TK_MACRO(Not, "not")
#endif

#if TK_INCLUDE & TK_KEYBINARY
 TK_MACRO(Is, "is")
 TK_MACRO(In, "in")
 TK_MACRO(And, "and")
 TK_MACRO(Or, "or")
 TK_MACRO(As, "as")
#endif

#if TK_INCLUDE & TK_KEYLITERAL
 TK_MACRO(True, "True")
 TK_MACRO(False, "False")
 TK_MACRO(None, "None")
#endif

#if TK_INCLUDE & TK_PRIBINARY
 TK_MACRO(Separator, "")
 TK_MACRO(AnonFn, "")
 TK_MACRO(NotIn, "")
 TK_MACRO(IsNot, "")
#endif

#if TK_INCLUDE & TK_PRIMARY
 TK_MACRO(Identifier, "")
 TK_MACRO(String, "")
# if TK_INCLUDE & TK_PRINUMBER
  TK_MACRO(Number, "")
# else
  TK_MACRO(Binary, "")
  TK_MACRO(Octal, "")
  TK_MACRO(Hexadecimal, "")
  TK_MACRO(Decimal, "")
  TK_MACRO(Floating, "")
# endif
#endif

#if TK_INCLUDE & TK_SINGLE
# if TK_INCLUDE & TK_FIRST
  TK_MACRO(NotEqual, '!')
# endif
# if TK_INCLUDE & TK_OPCMP
  TK_MACRO(Less, '<')
  TK_MACRO(Greater, '>')
# endif
# if TK_INCLUDE & TK_OPOTHER
  TK_MACRO(Dot, '.')
  TK_MACRO(Xor, '^')
  TK_MACRO(Plus, '+')
  TK_MACRO(Minus, '-')
  TK_MACRO(Star, '*')
  TK_MACRO(Percent, '%')
  TK_MACRO(Slash, '/')
  TK_MACRO(BitOr, '|')
  TK_MACRO(BitAnd, '&')
  TK_MACRO(Invert, '~')
  TK_MACRO(At, '@')
# endif
# else
# if TK_INCLUDE & TK_OPCMP
  TK_MACRO(NotEqual, "!=")
  TK_MACRO(EqualEqual, "==")
  TK_MACRO(LessEqual, "<=")
  TK_MACRO(GreaterEqual, ">=")
  TK_MACRO(Less, "<")
  TK_MACRO(Greater, ">")
# endif
# if TK_INCLUDE & TK_OPOTHER
  TK_MACRO(Dot, ".")
  TK_MACRO(Xor, "^")
  TK_MACRO(Lshift, "<<")
  TK_MACRO(Rshift, ">>")
  TK_MACRO(Plus, "+")
  TK_MACRO(Minus, "-")
  TK_MACRO(Star, "*")
  TK_MACRO(Percent, "%")
  TK_MACRO(Slash, "/")
  TK_MACRO(BitOr, "|")
  TK_MACRO(BitAnd, "&")
  TK_MACRO(Invert, "~")
  TK_MACRO(At, "@")
# endif
#endif

#if TK_INCLUDE & TK_ASSIGN
# if TK_INCLUDE & TK_SINGLE
  TK_MACRO(Equal, '=')
# else
  TK_MACRO(Equal, "=")
  TK_MACRO(XorEqual, "^=")
  TK_MACRO(PlusEqual, "+=")
  TK_MACRO(MinusEqual, "-=")
  TK_MACRO(StarEqual, "*=")
  TK_MACRO(PercentEqual, "%=")
  TK_MACRO(SlashEqual, "/=")
  TK_MACRO(LshiftEqual, "<<=")
  TK_MACRO(RshiftEqual, ">>=")
  TK_MACRO(BitOrEqual, "|=")
  TK_MACRO(BitAndEqual, "&=")
  TK_MACRO(InvertEqual, "~=")
  TK_MACRO(AtEqual, "@=")
# endif
#endif

#if TK_INCLUDE & TK_SYMBOL
# if TK_INCLUDE & TK_SINGLE
  TK_MACRO(Comma, ',')
  TK_MACRO(Colon, ':')
  TK_MACRO(HashTag, '#')
  TK_MACRO(OpenBrace, '{')
  TK_MACRO(CloseBrace, '}')
  TK_MACRO(OpenParen, '(')
  TK_MACRO(CloseParen, ')')
  TK_MACRO(OpenBracket, '[')
  TK_MACRO(CloseBracket, ']')
# else
  TK_MACRO(Comma, ",")
  TK_MACRO(Colon, ":")
  TK_MACRO(HashTag, "#")
  TK_MACRO(OpenBrace, "{")
  TK_MACRO(CloseBrace, "}")
  TK_MACRO(OpenParen, "(")
  TK_MACRO(CloseParen, ")")
  TK_MACRO(OpenBracket, "[")
  TK_MACRO(CloseBracket, "]")
# endif
#endif

#undef TK_INCLUDE
#undef TK_MACRO

