/* fileutils.c */
#include "fileutils.h"

/* Analyze a file and create metadata structure */
struct FileMetadata *analyzeFile(const char *filename) {
  struct FileMetadata *metadata;
  struct FileInfoBlock *fib;
  BPTR fh;
  BOOL success;
  char *lineStart;
  ULONG lineNum;
  ULONG filePos;
  struct TextLine *currentLine;
  struct TextLine *line;

  success = FALSE;
  metadata = AllocMem(sizeof(struct FileMetadata), MEMF_CLEAR);
  if (!metadata) return NULL;

  /* Copy filename and get full path */
  strncpy(metadata->filename, FilePart(filename), MAX_FILENAME_LEN - 1);
  strncpy(metadata->fullPath, filename, MAX_PATH_LEN - 1);

  /* Open the file */
  fh = Open(filename, MODE_OLDFILE);
  if (!fh) {
    FreeMem(metadata, sizeof(struct FileMetadata));
    return NULL;
  }

  /* Get file size */
  Seek(fh, 0, OFFSET_END);
  metadata->fileSize = Seek(fh, 0, OFFSET_BEGINNING);

  /* Read entire file into memory */
  metadata->fileData = AllocMem(metadata->fileSize + 1, MEMF_CLEAR);
  if (metadata->fileData) {
    if (Read(fh, metadata->fileData, metadata->fileSize) == metadata->fileSize) {
      success = TRUE;
    }
  }

  /* Get file protection bits */
  fib = AllocDosObject(DOS_FIB, NULL);
  if (fib) {
    if (ExamineFH(fh, fib)) {
      metadata->protection = fib->fib_Protection;
      metadata->dateStamp = fib->fib_Date;
    }
    FreeDosObject(DOS_FIB, fib);
  }

  Close(fh);

  if (!success) {
    freeFileMetadata(metadata);
    return NULL;
  }

  /* Determine if file is binary or text */
  metadata->isBinary = !isTextFile(metadata->fileData, metadata->fileSize);

  /* If text file, parse lines */
  if (!metadata->isBinary) {
    lineStart = metadata->fileData;
    lineNum = 1;
    filePos = 0;
    currentLine = NULL;

    while (lineStart < metadata->fileData + metadata->fileSize) {
      line = parseLine(lineStart, lineNum, filePos);

      if (!line) break;

      /* Link the line into our list */
      if (!metadata->lines) {
        metadata->lines = line;
      } else if (currentLine) {
        currentLine->next = line;
      }
      currentLine = line;

      filePos += line->rawLength;
      lineStart += line->rawLength;
      lineNum++;
      metadata->lineCount++;
    }
  }

  return metadata;
}

/* Determine if a file is text or binary */
BOOL isTextFile(const char *data, ULONG size) {
  ULONG i;
  ULONG nonPrintable;
  const ULONG threshold;
  unsigned char c;

  nonPrintable = 0;
  threshold = size / 10;  /* 10% threshold for binary determination */

  for (i = 0; i < size; i++) {
    c = data[i];
    if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
      nonPrintable++;
      if (nonPrintable > threshold) return FALSE;
    }
  }

  return TRUE;
}

/* Parse a single line of text */
struct TextLine *parseLine(const char *lineStart, ULONG lineNum, ULONG filePos) {
  struct TextLine *line;
  const char *lineEnd;
  ULONG len;
  ULONG rawLen;

  /* Find end of line */
  lineEnd = lineStart;
  while (*lineEnd && *lineEnd != '\n' && *lineEnd != '\r') lineEnd++;

  len = lineEnd - lineStart;
  if (len > MAX_LINE_LEN) len = MAX_LINE_LEN;

  /* Calculate raw length including newline characters */
  rawLen = len;
  if (*lineEnd == '\r' && *(lineEnd + 1) == '\n') {
    rawLen += 2;  /* CRLF */
  } else if (*lineEnd == '\n' || *lineEnd == '\r') {
    rawLen += 1;  /* LF or CR */
  }

  line = AllocMem(sizeof(struct TextLine), MEMF_CLEAR);
  if (!line) return NULL;

  line->content = AllocMem(len + 1, MEMF_CLEAR);
  if (!line->content) {
    FreeMem(line, sizeof(struct TextLine));
    return NULL;
  }

  /* Copy line content */
  strncpy(line->content, lineStart, len);
  line->content[len] = '\0';

  line->lineNumber = lineNum;
  line->length = len;
  line->filePosition = filePos;
  line->rawLength = rawLen;
  line->hasNewline = (*lineEnd == '\n' || *lineEnd == '\r');

  determineLineType(line);

  return line;
}

/* Determine the type of a line (for script files) */
void determineLineType(struct TextLine *line) {
  char *trimmed;

  trimmed = line->content;
  /* Skip leading whitespace */
  while (*trimmed && (*trimmed == ' ' || *trimmed == '\t')) trimmed++;

  if (!*trimmed) {
    line->type = LINE_EMPTY;
  } else if (*trimmed == ';') {
    line->type = LINE_COMMENT;
  } else {
    line->type = LINE_COMMAND;
  }
}

/* Free all allocated memory for file metadata */
void freeFileMetadata(struct FileMetadata *metadata) {
  struct TextLine *line;
  struct TextLine *next;

  if (!metadata) return;

  if (metadata->fileData) {
    FreeMem(metadata->fileData, metadata->fileSize + 1);
  }

  if (metadata->lines) {
    line = metadata->lines;
    while (line) {
      next = line->next;
      if (line->content) FreeMem(line->content, strlen(line->content) + 1);
      FreeMem(line, sizeof(struct TextLine));
      line = next;
    }
  }

  FreeMem(metadata, sizeof(struct FileMetadata));
}

/* Print file information */
void printFileInfo(const struct FileMetadata *metadata) {
  struct TextLine *line;

  if (!metadata) return;

  Printf("File: %s\n", metadata->filename);
  Printf("Path: %s\n", metadata->fullPath);
  Printf("Size: %ld bytes\n", metadata->fileSize);
  Printf("Type: %s\n", metadata->isBinary ? "Binary" : "Text");

  if (!metadata->isBinary) {
    Printf("Lines: %ld\n", metadata->lineCount);

    line = metadata->lines;
    while (line) {
      Printf("%4ld (@%08lx): [%s] %s\n",
        line->lineNumber,
        line->filePosition,
        line->type == LINE_EMPTY ? "EMPTY" :
        line->type == LINE_COMMENT ? "COMMENT" :
        line->type == LINE_COMMAND ? "COMMAND" : "UNKNOWN",
        line->content);
      line = line->next;
    }
  }
}

/* Wildcard pattern matching (supports * and ? wildcards) */
BOOL wildcardMatch(const char *pattern, const char *text) {
  while (*pattern != '\0' && *text != '\0') {
    if (*pattern == '*') {
      pattern++;
      /* Skip multiple asterisks */
      while (*pattern == '*') pattern++;

      if (*pattern == '\0') return TRUE;

      while (*text != '\0') {
        if (wildcardMatch(pattern, text)) return TRUE;
        text++;
      }
      return FALSE;
    }
    else if (*pattern == '?' || *pattern == *text) {
      pattern++;
      text++;
    }
    else return FALSE;
  }

  /* Skip remaining asterisks */
  while (*pattern == '*') pattern++;

  return (*pattern == '\0' && *text == '\0');
}

/* Find first line matching a wildcard pattern */
struct TextLine *findLineByPattern(const struct FileMetadata *metadata,
                                 const char *pattern, BOOL noCase) {
  struct TextLine *line;

  if (!metadata || !pattern || metadata->isBinary) return NULL;

  line = metadata->lines;
  while (line) {
    if (noCase ? MatchStringNoCase(pattern, line->content)
               : MatchString(pattern, line->content)) {
      return line;
    }
    line = line->next;
  }

  return NULL;
}

/* Insert a new line at the specified position (1-based) */
BOOL insertLine(struct FileMetadata *metadata, ULONG position, const char *content) {
  struct TextLine *newLine;
  struct TextLine *current;
  struct TextLine *prev;
  ULONG currentPos;

  if (!metadata || !content || metadata->isBinary || position < 1) return FALSE;

  /* Create new line structure */
  newLine = AllocMem(sizeof(struct TextLine), MEMF_CLEAR);
  if (!newLine) return FALSE;

  newLine->content = AllocMem(strlen(content) + 1, MEMF_CLEAR);
  if (!newLine->content) {
    FreeMem(newLine, sizeof(struct TextLine));
    return FALSE;
  }

  strcpy(newLine->content, content);
  newLine->length = strlen(content);
  newLine->rawLength = newLine->length + 1; /* Assume single newline */
  newLine->hasNewline = TRUE;
  determineLineType(newLine);

  /* Handle insertion at beginning */
  if (position == 1) {
    newLine->next = metadata->lines;
    metadata->lines = newLine;
    metadata->lineCount++;
    return TRUE;
  }

  /* Find insertion point */
  prev = NULL;
  current = metadata->lines;
  currentPos = 1;

  while (current && currentPos < position) {
    prev = current;
    current = current->next;
    currentPos++;
  }

  /* Insert the new line */
  if (prev) {
    prev->next = newLine;
    newLine->next = current;
    metadata->lineCount++;
    return TRUE;
  }

  /* Position was beyond end of file */
  FreeMem(newLine->content, strlen(content) + 1);
  FreeMem(newLine, sizeof(struct TextLine));
  return FALSE;
}

/* Remove line by line number (1-based) */
BOOL removeLine(struct FileMetadata *metadata, ULONG lineNumber) {
  struct TextLine *current;
  struct TextLine *prev;
  ULONG currentPos;

  if (!metadata || metadata->isBinary || lineNumber < 1 ||
      lineNumber > metadata->lineCount) {
    return FALSE;
  }

  prev = NULL;
  current = metadata->lines;
  currentPos = 1;

  /* Find the line to remove */
  while (current && currentPos < lineNumber) {
    prev = current;
    current = current->next;
    currentPos++;
  }

  if (!current) return FALSE;

  /* Remove the line */
  if (prev) {
    prev->next = current->next;
  } else {
    metadata->lines = current->next;
  }

  /* Free the removed line */
  FreeMem(current->content, current->length + 1);
  FreeMem(current, sizeof(struct TextLine));

  metadata->lineCount--;
  return TRUE;
}

/* Remove first line matching pattern */
BOOL removeLineByPattern(struct FileMetadata *metadata, const char *pattern) {
  struct TextLine *current;
  struct TextLine *prev;

  if (!metadata || !pattern || metadata->isBinary) return FALSE;

  prev = NULL;
  current = metadata->lines;

  while (current) {
    if (wildcardMatch(pattern, current->content)) {
      if (prev) {
        prev->next = current->next;
      } else {
        metadata->lines = current->next;
      }

      FreeMem(current->content, current->length + 1);
      FreeMem(current, sizeof(struct TextLine));
      metadata->lineCount--;
      return TRUE;
    }
    prev = current;
    current = current->next;
  }

  return FALSE;
}

/* Save current state to a new file */
BOOL saveToFile(const struct FileMetadata *metadata, const char *outputPath) {
  BPTR file;
  struct TextLine *line;
  const char newline = '\n';
  BOOL success;

  if (!metadata || !outputPath) return FALSE;

  success = TRUE;
  file = Open(outputPath, MODE_NEWFILE);
  if (!file) return FALSE;

  if (metadata->isBinary) {
    /* Write binary data directly */
    if (Write(file, metadata->fileData, metadata->fileSize) != metadata->fileSize) {
      success = FALSE;
    }
  } else {
    /* Write text data line by line */
    line = metadata->lines;
    while (line && success) {
      if (Write(file, line->content, line->length) != line->length) {
        success = FALSE;
        break;
      }

      if (line->hasNewline && Write(file, &newline, 1) != 1) {
        success = FALSE;
        break;
      }

      line = line->next;
    }
  }

  Close(file);
  return success;
}
