#ifndef STREAM_H
#define STREAM_H

//C
#include <stdio.h>
#include <stdlib.h>

// CUT
#include <diagnostic.h>
#include <oop.h>
#include <map.h>


#define TYPENAME JSON

// (!) This class must always be initialized using the NEW macro
OBJECT (const char *filename) INHERIT (Map)
  const char *filename;
END(NULL);

#undef TYPENAME
#endif