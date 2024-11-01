#ifndef JSON_H
#define JSON_H

//C
#include <stdio.h>
#include <stdlib.h>

// CUT
#include <diagnostic.h>
#include <oop.h>
#include <map.h>
#include <charstream.h>
#include <str.h>


#define TYPENAME JSON

// (!) This class must always be initialized using the NEW macro
OBJECT () INHERIT (Map)
END_OBJECT();

void _(Deserialize)(CharStream *stream);
void _(Serialize)(CharStream *stream);

#undef TYPENAME

#define TYPENAME JSONException

OBJECT (const char *message) INHERIT (Exception)
END_OBJECT("");

#undef TYPENAME
#endif