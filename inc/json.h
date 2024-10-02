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
END();

void _(deserialize)(CharStream *stream);
void _(serialize)(CharStream *stream);

#undef TYPENAME
#endif