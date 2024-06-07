#ifdef TOKEN_KIND

TOKEN_KIND(Error) // symbolyze an error like invalid character
TOKEN_KIND(Eot) // end of tokens marker

TOKEN_KIND(Raise)
TOKEN_KIND(Class)
TOKEN_KIND(Fn)
TOKEN_KIND(Let)
TOKEN_KIND(As)
TOKEN_KIND(Import)
TOKEN_KIND(In)
TOKEN_KIND(Not)
TOKEN_KIND(And)
TOKEN_KIND(Or)
TOKEN_KIND(Try)
TOKEN_KIND(Else)
TOKEN_KIND(If)
TOKEN_KIND(While)
TOKEN_KIND(For)
TOKEN_KIND(Lastly)
TOKEN_KIND(Except)
TOKEN_KIND(Return)
TOKEN_KIND(With)
TOKEN_KIND(Assert)
TOKEN_KIND(From)
TOKEN_KIND(True)
TOKEN_KIND(False)

TOKEN_KIND(Comma) // ,
TOKEN_KIND(HashTag) // #
TOKEN_KIND(Semicolon) // ;
TOKEN_KIND(OpenBrace) // {
TOKEN_KIND(CloseBrace) // }
TOKEN_KIND(OpenParen) // (
TOKEN_KIND(CloseParen) // )
TOKEN_KIND(OpenBracket) // [
TOKEN_KIND(CloseBracket) // ]

TOKEN_KIND(Identifier) // [a-zA-Z_][a-zA-Z0-9_]*
TOKEN_KIND(String) // 'single line string' "single line string"
TOKEN_KIND(LongString) // """Multiline String"""
TOKEN_KIND(Binary) // 0b101010101
TOKEN_KIND(Octal) // 0o215465436
TOKEN_KIND(Decimal) // 63655645783
TOKEN_KIND(Hexadecimal) // 0x6ab5cd6754ef
TOKEN_KIND(Floating) // 123.45 6.789e4 45e-3

TOKEN_KIND(Dot) // 
TOKEN_KIND(Equal) // =
TOKEN_KIND(NotEqual) // !=
TOKEN_KIND(EqualEqual) // ==
TOKEN_KIND(Plus) // +
TOKEN_KIND(PlusEqual) // +=
TOKEN_KIND(Minus) // -
TOKEN_KIND(MinusEqual)// -=
TOKEN_KIND(Star) // *
TOKEN_KIND(StarEqual) // *=
TOKEN_KIND(Percent) // %
TOKEN_KIND(PercentEqual) // %=
TOKEN_KIND(Slash) // /
TOKEN_KIND(SlashEqual) // /=
TOKEN_KIND(Less) // <
TOKEN_KIND(LessEqual) // <=
TOKEN_KIND(Greater) // >
TOKEN_KIND(GreaterEqual) // >=
TOKEN_KIND(Lshift) // <<
TOKEN_KIND(LshiftEqual) // <<=
TOKEN_KIND(Rshift) // >>
TOKEN_KIND(RshiftEqual) // >>=

#else
# warning "Did you forget to define `TOKEN_KIND(tok_kind)` function macro"
#endif
