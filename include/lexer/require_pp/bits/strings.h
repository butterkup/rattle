#include "ensure_def.h"

#ifndef rattle_strings_h
# define rattle_strings_h

TOKEN_MACRO(MultilineString, "")  // """Long\nString""" '''Also\nLong\nString'''
TOKEN_MACRO(SingleLineString, "") // "One line string" 'One line string'

#endif
