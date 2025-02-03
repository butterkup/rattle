#ifndef TOKEN_MACRO
# error "macro TOKEN_MACRO(arg1, arg2) was not defined"
# define TOKEN_MACRO(k, s)
#endif

// Ensures TOKEN_MACRO as it is needed by all other files in this
// directory for preprocessing.
