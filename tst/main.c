#include <stdio.h>

#include <diagnostic.h>
#include <jsonfile.h>
#include <oop.h>

int main(void)
{
  CHECK_MEMORY

  JSONFile *json = NEW (JSONFile) ("tst/test.json", ACCESS_READ | ACCESS_WRITE);

// TODO: Check with read-to-end

  CHECK_MEMORY

  DELETE (json);

  CHECK_MEMORY

  STOP_WATCHING
}