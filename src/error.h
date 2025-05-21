# ifndef ERROR_H
# define ERROR_H

#include <stdarg.h>

extern char *user_input;

void error_at(char *loc, char *fmt, ...);

# endif
