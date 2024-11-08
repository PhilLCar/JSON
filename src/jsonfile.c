#include <jsonfile.h>

#define TYPENAME JSONFile

////////////////////////////////////////////////////////////////////////////////
JSONFile *_(Construct)(const char *filename, FileAccessModes mode)
{  
  if (JSON_Construct(BASE(0))) {
    this->filename = malloc(strlen(filename) + 1);
    this->mode     = mode;

    strcpy((void*)this->filename, filename);

    if (mode & FILEACCESS_READ) {
      CharStream *stream = (CharStream*) NEW (FileStream) (fopen(filename, "r"));

      if (stream) {
        JSON_Deserialize(BASE(0), stream);
        DELETE (stream);
      } else {
        THROW(NEW (Exception)("File not found!"));
      }
    }
  }

  return this;
}

////////////////////////////////////////////////////////////////////////////////
void _(Destruct)()
{
  if (this) {
    if (this->mode & FILEACCESS_WRITE) {
      CharStream *stream = (CharStream*) NEW (FileStream) (fopen(this->filename, "w+"));

      if (stream) {
        JSON_Serialize(BASE(0), stream);
        DELETE (stream);
      } else {
        THROW(NEW (Exception)("Couldn't open file!"));
      }
    }

    free((void*)this->filename);
    this->filename = NULL;
    
    JSON_Destruct(BASE(0));
  }
}

#undef TYPENAME
