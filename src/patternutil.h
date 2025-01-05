/* patternutil.h */
#ifndef PATTERNUTIL_H
#define PATTERNUTIL_H

#include <exec/types.h>
#include <dos/dos.h>
#include <proto/dos.h>

/* Pattern matching convenience functions */
BOOL MatchString(CONST_STRPTR pattern, STRPTR string);
BOOL MatchStringNoCase(CONST_STRPTR pattern, STRPTR string);

#endif