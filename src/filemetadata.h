#ifndef FILEMETADATA_H
#define FILEMETADATA_H

#include "textline.h"

/* Main structure for file metadata and content */
struct FileMetadata {
  char filename[MAX_FILENAME_LEN];  /* File name */
  char fullPath[MAX_PATH_LEN];      /* Full path */
  ULONG fileSize;                   /* Size in bytes */
  ULONG protection;                 /* AmigaDOS protection bits */
  BOOL isBinary;                    /* Binary or text flag */
  char *fileData;                   /* Raw file data */
  struct DateStamp dateStamp;       /* File date stamp */

  /* Text file specific data */
  struct TextLine *lines;           /* Array of line structures */
  ULONG lineCount;                  /* Number of lines */
};

/* File analysis functions */
struct FileMetadata *analyzeFile(const char *filename);
void freeFileMetadata(struct FileMetadata *metadata);
BOOL isTextFile(const char *data, ULONG size);
void printFileInfo(const struct FileMetadata *metadata);

#endif