#include "ensure_def.h"

#ifndef rattle_strings_h
# define rattle_strings_h

rattle_pp_token_macro(MultilineString, "")  // """Long\nString""" '''Also\nLong\nString'''
rattle_pp_token_macro(SingleLineString, "") // "One line string" 'One line string'

#endif
