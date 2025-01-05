/* fileutils.h */
#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <exec/types.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <stdlib.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#define MAX_FILENAME_LEN 108  /* AmigaDOS max filename length */
#define MAX_PATH_LEN    256   /* Reasonable path length limit */
#define MAX_LINE_LEN    1024  /* Maximum line length for text files */
#define PATTERN_NOMATCH 0     /* Pattern does not match */
#define PATTERN_MATCH   1     /* Pattern matches */

#include "linetype.h"
#include "filetype.h"
#include "filemetadata.h"
#include "textline.h"

/* Pattern matching and line manipulation functions */

BOOL wildcardMatch(const char *pattern, const char *text);
BOOL insertLine(struct FileMetadata *metadata, ULONG position, const char *content);
BOOL removeLine(struct FileMetadata *metadata, ULONG lineNumber);
BOOL removeLineByPattern(
  struct FileMetadata *metadata,
  const char *pattern,
  BOOL noCase
);
BOOL saveToFile(const struct FileMetadata *metadata, const char *outputPath);

#endif /* FILEUTILS_H */
