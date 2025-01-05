/* analyze.c */
#include "fileutils.h"
#include <workbench/startup.h>
#include <proto/icon.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>

/* Global variables for Amiga CLI/Workbench startup */
int _WBargc;
char **_WBargv;

/* Argument template */
const char *TEMPLATE = "HELP/S,COMMAND/A,FILE/A,PATTERN/K,LINE/N,TEXT/K,OUTPUT/K";
const char *VERSTAG = "\0$VER: Analyze 1.0 (1.1.2025)\0";

enum {
  ARG_HELP,
  ARG_COMMAND,
  ARG_FILE,
  ARG_PATTERN,
  ARG_LINE,
  ARG_TEXT,
  ARG_OUTPUT,
  TOTAL_ARGS
};

void printUsage(void) {
  Printf("\nAnalyze - Text file manipulation utility\n");
  Printf("© 2025 Your Name\n\n");
  Printf("FORMAT:\n");
  Printf("  ANALYZE COMMAND FILE [PATTERN pattern] [LINE n] [TEXT string] [OUTPUT file]\n\n");
  Printf("COMMAND:\n");
  Printf("  INFO    - Show file information\n");
  Printf("  FIND    - Find lines matching pattern\n");
  Printf("  INSERT  - Insert a line at position\n");
  Printf("  DELETE  - Delete a line by number\n");
  Printf("  REMOVE  - Remove lines matching pattern\n");
  Printf("  REPLACE - Replace line(s) with new text\n");
  Printf("  SAVE    - Save modifications to new file\n\n");
  Printf("ARGUMENTS:\n");
  Printf("  FILE    - Source file to analyze\n");
  Printf("  PATTERN - Pattern to match (* and ? wildcards supported)\n");
  Printf("  LINE    - Line number for operations\n");
  Printf("  TEXT    - Text content for insert/replace\n");
  Printf("  OUTPUT  - Destination file for save\n\n");
  Printf("EXAMPLE:\n");
  Printf("  ANALYZE INFO \"script.txt\"\n");
  Printf("  ANALYZE FIND \"script.txt\" PATTERN \"echo *\"\n");
  Printf("  ANALYZE INSERT \"script.txt\" LINE 5 TEXT \"echo \\\"Hello\\\"\"\n");
  Printf("  ANALYZE REPLACE \"script.txt\" PATTERN \"echo *\" TEXT \"print \\\"Hello\\\"\"\n");
  Printf("  ANALYZE SAVE \"script.txt\" OUTPUT \"script.new\"\n");
}

/* Execute the requested command */
LONG executeCommand(const char *command, struct FileMetadata *metadata,
                   STRPTR pattern, LONG *line, STRPTR text, STRPTR output) {
  struct TextLine *foundLine;
  BOOL success;

  if (stricmp(command, "INFO") == 0) {
    printFileInfo(metadata);
    return RETURN_OK;
  }

  if (stricmp(command, "FIND") == 0) {
    if (!pattern) {
      Printf("PATTERN argument required for FIND command\n");
      return RETURN_ERROR;
    }

    foundLine = findLineByPattern(metadata, pattern);
    if (foundLine) {
      Printf("Found at line %ld: %s\n", foundLine->lineNumber, foundLine->content);
    } else {
      Printf("Pattern not found\n");
    }
    return RETURN_OK;
  }

  if (stricmp(command, "INSERT") == 0) {
    if (!line || !text) {
      Printf("LINE and TEXT arguments required for INSERT command\n");
      return RETURN_ERROR;
    }

    if (insertLine(metadata, *line, text)) {
      Printf("Line inserted\n");
      return RETURN_OK;
    } else {
      Printf("Failed to insert line\n");
      return RETURN_ERROR;
    }
  }

  if (stricmp(command, "DELETE") == 0) {
    if (!line) {
      Printf("LINE argument required for DELETE command\n");
      return RETURN_ERROR;
    }

    if (removeLine(metadata, *line)) {
      Printf("Line deleted\n");
      return RETURN_OK;
    } else {
      Printf("Failed to delete line\n");
      return RETURN_ERROR;
    }
  }

  if (stricmp(command, "REMOVE") == 0) {
    if (!pattern) {
      Printf("PATTERN argument required for REMOVE command\n");
      return RETURN_ERROR;
    }

    if (removeLineByPattern(metadata, pattern)) {
      Printf("Line removed\n");
      return RETURN_OK;
    } else {
      Printf("Pattern not found or removal failed\n");
      return RETURN_ERROR;
    }
  }

  if (stricmp(command, "REPLACE") == 0) {
    /* Replace can work with either LINE or PATTERN */
    if ((!line && !pattern) || !text) {
      Printf("TEXT and either LINE or PATTERN required for REPLACE command\n");
      return RETURN_ERROR;
    }

    /* Prioritize LINE if both are specified */
    if (line) {
      if (removeLine(metadata, *line) && insertLine(metadata, *line, text)) {
        Printf("Line replaced\n");
        return RETURN_OK;
      }
    } else {
      if (removeLineByPattern(metadata, pattern) &&
          insertLine(metadata, 1, text)) {
        Printf("Line replaced\n");
        return RETURN_OK;
      }
    }
    Printf("Failed to replace line\n");
    return RETURN_ERROR;
  }

  if (stricmp(command, "SAVE") == 0) {
    if (!output) {
      Printf("OUTPUT argument required for SAVE command\n");
      return RETURN_ERROR;
    }

    success = saveToFile(metadata, output);
    if (success) {
      Printf("File saved to %s\n", output);
      return RETURN_OK;
    } else {
      Printf("Failed to save file\n");
      return RETURN_ERROR;
    }
  }

  Printf("Unknown command: %s\n", command);
  return RETURN_ERROR;
}

int main(int argc, char *argv[]) {
  struct RDArgs *rdargs;
  struct FileMetadata *metadata;
  LONG result;
  LONG args[TOTAL_ARGS] = {0};

  /* Handle Workbench startup */
  if (argc == 0) {
    Printf("Program must be started from CLI\n");
    return RETURN_FAIL;
  }

  /* Parse arguments */
  rdargs = ReadArgs(TEMPLATE, args, NULL);
  if (!rdargs) {
    PrintFault(IoErr(), argv[0]);
    return RETURN_ERROR;
  }

  /* Check for help request */
  if (args[ARG_HELP]) {
    FreeArgs(rdargs);
    printUsage();
    return RETURN_OK;
  }

  /* Load and analyze the file */
  metadata = analyzeFile((STRPTR)args[ARG_FILE]);
  if (!metadata) {
    Printf("Could not analyze file %s\n", (STRPTR)args[ARG_FILE]);
    FreeArgs(rdargs);
    return RETURN_ERROR;
  }

  /* Execute the requested command */
  result = executeCommand(
    (STRPTR)args[ARG_COMMAND],
    metadata,
    (STRPTR)args[ARG_PATTERN],
    (LONG *)args[ARG_LINE],
    (STRPTR)args[ARG_TEXT],
    (STRPTR)args[ARG_OUTPUT]
  );

  /* Clean up */
  freeFileMetadata(metadata);
  FreeArgs(rdargs);

  return result;
}
