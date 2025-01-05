#ifndef TEXTLINE_H
#define TEXTLINE_H

/* Forward declarations */
struct FileMetadata;

/* Structure to represent a single line in a text file */
struct TextLine {
  struct FileMetadata *parent; /* The file this line belongs to */
  char *content;               /* The actual line content */
  ULONG lineNumber;            /* Line number in file (1-based) */
  ULONG length;                /* Length of the line */
  ULONG filePosition;          /* Position in file where line starts */
  ULONG rawLength;             /* Length including newline chars */
  BOOL hasNewline;             /* Whether line ends with newline */
  LineType type;               /* Type of line (for script files) */
  struct TextLine *next;       /* Pointer to next line (if needed) */
};

struct TextLine *parseLine(const char *lineStart, ULONG lineNum, ULONG filePos);
struct TextLine *findLineByPattern(
  const struct FileMetadata *metadata,
  const char *pattern, BOOL noCase
);

#endif