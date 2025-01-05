#include "textline.h"
#include "filemetadata.h"
#include "linetype.h"

LineType determineLineType(struct TextLine *line, AnalyzeLine **analyzers) {
  AnalyzeLine *analyzer;

  if (!line) {
    return LINE_UNKNOWN;
  }

  while (analyzer = *analyzers) {
    LineType type = analyzer(line, line->parent);
    analyzer++;

    if (!analyzer) {
      return type
    }
  }

  return LINE_UNKNOWN;
};

AnalyzeLine *DetectADOSScriptComment(TextLine *line) {
  char *trimmed;

  trimmed = line->content;
  /* Skip leading whitespace */
  while (*trimmed && (*trimmed == ' ' || *trimmed == '\t')) trimmed++;

  if (*trimmed == ';') {
    return LINE_COMMENT;
  }

  return LINE_UNKNOWN;
}

AnalyzeLine *DetectADOSScriptCommand(TextLine *line) {
  char *trimmed;

  trimmed = line->content;
  /* Skip leading whitespace */
  while (*trimmed && (*trimmed == ' ' || *trimmed == '\t')) trimmed++;

  if (!*trimmed && *trimmed != ';') {
    return LINE_COMMAND;
  }

  /* Maybe also check if the command exists within the path for a smarter
     approach */

  return LINE_UNKNOWN;
}

AnalyzeLine *DetectADOSEmptyLine(TextLine *line) {
  char *trimmed;

  trimmed = line->content;
  /* Skip leading whitespace */
  while (*trimmed && (*trimmed == ' ' || *trimmed == '\t')) trimmed++;

  if (!*trimmed) {
    return LINE_EMPTY;
  }

  return LINE_UNKNOWN;
}