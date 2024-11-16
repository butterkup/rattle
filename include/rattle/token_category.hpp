#pragma once

#define TK_KEYWORD  0x01
#define TK_SYMBOL   0x02
#define TK_SPECIAL  0x04
#define TK_OPOTHER  0x08
#define TK_OPCMP    0x10
#define TK_ASSIGN   0x20
#define TK_PRIMARY  0x40

#define TK_OPALL    (TK_OPOTHER | TK_OPCMP)
#define TK_ALL (2 * TK_PRIMARY - 1)

