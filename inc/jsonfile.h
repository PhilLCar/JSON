#ifndef JSONFILE_H
#define JSONFILE_H

//C
#include <stdio.h>
#include <stdlib.h>

// CUT
#include <diagnostic.h>
#include <filestream.h>
#include <json.h>

#define TYPENAME JSONFile

// (!) This class must always be initialized using the NEW macro
OBJECT(const char *filename, int readonly) INHERIT (JSON)
  const char *filename;
  int         readonly;
END_OBJECT(NULL, 1);

#undef TYPENAME
#endif