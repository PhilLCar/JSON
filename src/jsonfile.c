#include <jsonfile.h>

#define TYPENAME JSONFile

////////////////////////////////////////////////////////////////////////////////
JSONFile *_(Construct)(const char *filename)
{  
  if (JSON_Construct(BASE(0))) {
    CharStream *stream = (CharStream*) NEW (FileStream) (fopen(filename, "r"));

    if (stream) {
      this->filename = filename;
      this->flush    = 1; // <= default

      JSON_Deserialize(BASE(0), stream);
      DELETE (stream);
    } else {
      THROW(NEW (Exception)("File not found!"));
    }
  }

  return this;
}

////////////////////////////////////////////////////////////////////////////////
void _(Destruct)()
{
  if (this) {
    if (this->flush) {
      CharStream *stream = (CharStream*) NEW (FileStream) (fopen(this->filename, "w+"));

      if (stream) {
        JSON_Serialize(BASE(0), stream);
        DELETE (stream);
      } else {
        THROW(NEW (Exception)("Couldn't open file!"));
      }
    }

    JSON_Destruct(BASE(0));
  }
}

#undef TYPENAME
