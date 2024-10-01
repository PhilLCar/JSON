#include <stdio.h>

#include <diagnostic.h>
#include <json.h>
#include <oop.h>

int main(void)
{
  CHECK_MEMORY

  JSON *json = NEW (JSON) ();

  CHECK_MEMORY

  CharStream *file = (CharStream*) NEW (FileStream) ("tst/test.json", "r");

  JSON_deserialize(json, file);

  DELETE (file);

  CHECK_MEMORY

  file = (CharStream*) NEW (FileStream) ("tst/output.json", "w+");

  CHECK_MEMORY

  JSON_serialize(json, file);

  CHECK_MEMORY

  DELETE (file);
  DELETE (json);

  CHECK_MEMORY

  STOP_WATCHING
}