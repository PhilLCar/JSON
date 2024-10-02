#include <jsonfile.h>

#define TYPENAME JSONFile

////////////////////////////////////////////////////////////////////////////////
JSONFile *_(cons)(const char *filename)
{  
  if (this) {
    JSON_cons(BASE(0));
    this->filename = filename;

    {
      CharStream *stream = (CharStream*) NEW (FileStream) (fopen(filename, "r"));

      if (stream) {
        JSON_deserialize(BASE(0), stream);
        DELETE (stream);
      }
    }
  }

  return this;
}

////////////////////////////////////////////////////////////////////////////////
void _(free)()
{
  CharStream *stream = (CharStream*) NEW (FileStream) (fopen(this->filename, "w+"));

  if (stream) {
    JSON_serialize(BASE(0), stream);
    DELETE (stream);
  }

  JSON_free(BASE(0));
}
