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
OBJECT(const char *filename, FileAccessModes mode) INHERIT (JSON)
  const char      *filename;
  FileAccessModes  mode;
END_OBJECT(NULL, FILEACCESS_READ);

#undef TYPENAME
#endif