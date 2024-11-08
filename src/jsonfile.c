#include <jsonfile.h>

#define TYPENAME JSONFile

////////////////////////////////////////////////////////////////////////////////
JSONFile *_(Construct)(const char *filename, int readonly)
{  
  if (JSON_Construct(BASE(0))) {
    CharStream *stream = (CharStream*) NEW (FileStream) (fopen(filename, "r"));

    if (stream) {
      this->filename = malloc(strlen(filename) + 1);
      this->readonly = readonly;

      strcpy((void*)this->filename, filename);
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
    if (!this->readonly) {
      CharStream *stream = (CharStream*) NEW (FileStream) (fopen(this->filename, "w+"));

      if (stream) {
        JSON_Serialize(BASE(0), stream);
        DELETE (stream);
      } else {
        THROW(NEW (Exception)("Couldn't open file!"));
      }

    }

    if (this->filename) {
      free((void*)this->filename);
      this->filename = NULL;
    }
    
    JSON_Destruct(BASE(0));
  }
}

#undef TYPENAME
