#ifndef FILETYPE_H
#define FILETYPE_H

/** Forward declaration */
struct FileMetadata;

/**
 * \brief The FileType struct provides some metadata around the type of file
 * currently being analyzed. This information can be used to determine which
 * type of context analyzers should be employed if the type is not unknown.
 *
 * - \c FILE_UNKNOWN a file type that could not be reliably detected at the
 * time the analysis was performed.
 * - \c FILE_ADOS_SCRIPT an AmigaDOS Shell script
 * - \c FILE_TEXT a text document of undetermined type (non-binary)
 * - \c FILE_BINARY a file that contains binary, non-printable, characters
 */
typedef enum FileType {
  FILE_UNKNOWN,
  FILE_ADOS_SCRIPT,
  FILE_TEXT,
  FILE_BINARY
} FileType;

typedef FileType (*FileTypeAnalyzer)(struct FileMetadata *metadata);

const FileTypeAnalyzer *ADOSShellScriptIdentifier;

#endif