/* patternutil.c */
#include "patternutil.h"

BOOL MatchString(CONST_STRPTR pattern, STRPTR string) {
  UBYTE *parsedPattern;
  BOOL result;
  LONG size;

  /* Get required buffer size */
  size = ParsePatternNoCase(pattern, NULL, 0);
  if (size < 0) return FALSE;

  /* Allocate pattern buffer */
  parsedPattern = AllocMem(size, MEMF_CLEAR);
  if (!parsedPattern) return FALSE;

  /* Parse and match */
  if (ParsePattern(pattern, parsedPattern, size) >= 0) {
    result = MatchPattern(parsedPattern, string);
  } else {
    result = FALSE;
  }

  FreeMem(parsedPattern, size);
  return result;
}

BOOL MatchStringNoCase(CONST_STRPTR pattern, STRPTR string) {
  UBYTE *parsedPattern;
  BOOL result;
  LONG size;

  /* Get required buffer size */
  size = ParsePatternNoCase(pattern, NULL, 0);
  if (size < 0) return FALSE;

  /* Allocate pattern buffer */
  parsedPattern = AllocMem(size, MEMF_CLEAR);
  if (!parsedPattern) return FALSE;

  /* Parse and match */
  if (ParsePatternNoCase(pattern, parsedPattern, size) >= 0) {
    result = MatchPatternNoCase(parsedPattern, string);
  } else {
    result = FALSE;
  }

  FreeMem(parsedPattern, size);
  return result;
}
