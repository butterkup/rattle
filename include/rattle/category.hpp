#pragma once

#define LEXER_ERROR 0x01

#define TK_KEYWORD  0x01
#define TK_SYMBOL   0x02
#define TK_SPECIAL  0x04
#define TK_OPS      0x08
#define TK_ASSIGN   0x10
#define TK_PRIMARY  0x20
#define TK_SINGLE   0x40

#define TK_SINGLEALL (TK_OPS | TK_ASSIGN | TK_SYMBOL | TK_SINGLE)
#define TK_OPALL    (TK_OPOTHER | TK_OPCMP)
#define TK_ALL      0xFF

