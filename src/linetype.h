#ifndef LINETYPE_H
#define LINETYPE_H

/* Forward declarations */
struct TextLine;
struct FileMetadata;

/* Enum for script line types */
typedef enum LineType {
  LINE_UNKNOWN,
  LINE_EMPTY,
  LINE_COMMENT,  /* Starts with semicolon or is empty */
  LINE_COMMAND   /* Actual AmigaDOS command */
} LineType;

/*!
 * The AnalyzeLine functions are used to determine if a line of a given
 * file is a comment, a command or some other type.
 *
 * \param line a pointer to a \see struct TextLine. It represents the line
 * in question
 * \param metadata a pointer to a \see struct FileMetadata. It represents
 * the whole file in question.
 */
typedef LineType (*AnalyzeLine)(struct TextLine *line);

LineType determineLineType(struct TextLine *line, AnalyzeLine **analyzers);

const AnalyzeLine *DetectADOSScriptComment;
const AnalyzeLine *DetectADOSScriptCommand;
const AnalyzeLine *DetectADOSEmptyLine;

const AnalyzeLine ADOSScriptAnalyzers[] = [
  DetectADOSScriptComment,
  DetectADOSScriptCommand,
  DetectADOSEmptyLine,
  NULL
];

#endf