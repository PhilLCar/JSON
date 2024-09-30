#include <json.h>

#define TYPENAME JSON

OBJECTIFY(double);

/******************************************************************************/
void STATIC (except) (const char *message)
{
  fprintf("%s\n", message);
  exit(-1);
}

/******************************************************************************/
char STATIC (skipws)(CharStream *stream)
{
  char c;

  while ((c = speek(stream)) != EOF) {
    switch (c) {
      case '\n':
      case '\r':
      case ' ':
      case '\t':
        sgetc(stream);
        continue;
      default:
        break;
    }
    break;
  }

  return c;
}

/******************************************************************************/
char STATIC (indent)(CharStream *stream, int indent)
{
  for (int i = 0; i < indent << 1; i++) {
    CharStream_put(stream, ' ');
  }
}

////////////////////////////////////////////////////////////////////////////////
JSON *_(cons)()
{  
  if (this) {
    Map_cons(BASE(0), OBJECT_TYPE(String), NATIVE_TYPE(void*), (Comparer)String_cequals);
  }

  return this;
}

////////////////////////////////////////////////////////////////////////////////
void _(free)()
{
  // TODO:
  for (int i = 0; i < BASE(2)->size; i++) {

  }
}

/******************************************************************************/
void *STATIC (any)(CharStream *stream)
{
  char c = JSON_skipws(stream);

  switch (c) {
    case '{':
      return JSON_dictionary(stream);
    case '[':
      return JSON_list(stream);
    case '"':
      return JSON_text(stream);
    default:
      return JSON_number(stream);
  }
}

/******************************************************************************/
Map *STATIC (dictionary)(CharStream *stream)
{
  Map *dictionary = NEW (Map) (OBJECT_TYPE(String), NATIVE_TYPE(void*), (Comparer)String_cequals);

  char c = JSON_skipws(stream);

  if (c == '{') {
    sgetc(stream);

    while ((c = JSON_skipws(stream)) != EOF) {
      if (c == '}') break;

      String *key = JSON_text(stream);

      if (JSON_skipws(stream) == ':') {
        sgetc(stream);
      } else {
        char error[256];
        sprintf("Expecting ':' after key '%s'!", key->base);
        JSON_except(error);
      }

      void *value = JSON_any(stream);

      Map_setkey(dictionary, key, &value);

      if ((c = JSON_skipws(stream)) == ',') {
        sgetc(stream);
      } else break;
    }

    if (sgetc(stream) != '}') {
      JSON_except("Expecting '}' at the end of a dictonary (look for a missing ',').");
    }

  } else {
    JSON_except("Expecting '{' at the begining of a dictonary.");
  }

  if (c == EOF) {
    JSON_except("Reached EOF before deserialization was over!");
  }

  return dictionary;
}

/******************************************************************************/
Array *STATIC (list)(CharStream *stream)
{
  Array *list = NEW (Array) (sizeof(void*));

  char c = JSON_skipws(stream);

  if (c == '[') {
    sgetc(stream);

    while ((c = JSON_skipws(stream)) != EOF) {
      if (c == ']') break;

      void *value = JSON_any(stream);

      Array_push(list, &value);

      if ((c = JSON_skipws(stream)) == ',') {
        sgetc(stream);
      } else break;
    }

    if (sgetc(stream) != ']') {
      JSON_except("Expecting ']' at the end of a list (look for a missing ',').");
    }
  } else {
    JSON_except("Expecting '[' at the begining of a list.");
  }
  
  if (c == EOF) {
    JSON_except("Reached EOF before deserialization was over!");
  }

  return list;
}

/******************************************************************************/
String *STATIC (text)(CharStream *stream)
{
  String *text = NEW (String) ("");
  char c = JSON_skipws(stream);

  if (c == '"') {
    sgetc(stream);

    while ((c = sgetc(stream)) != EOF) {
      switch (c) {
        case '"':
          break;
        case '\\':
          String_append(text, sesc(stream));
          continue;
        default:
          String_append(text, c);
          continue;
      }
      break;
    }
  } else {
    JSON_except("Expecting '\"' at the begining of a string.");
  }

  if (c == EOF) {
    JSON_except("Reached EOF before deserialization was over!");
  }

  return text;
}

/******************************************************************************/
double *STATIC (number)(CharStream *stream)
{
  String *digits = NEW (String) ("");
  double *number = talloc(&OBJECT_TYPE(double));
  char    c;

  JSON_skipws(stream);

  // For now numbers are going to be decimal floating points only
  while ((c = speek(stream)) != EOF) {
    if ((c >= '0' && c <= '9') || c == '.') {
      String_append(digits, sgetc(stream));
    } else break;
  }

  if (c == EOF) {
    JSON_except("Reached EOF before deserialization was over!");
  }

  *number = atof(digits->base);

  DELETE (digits);

  return number;
}

////////////////////////////////////////////////////////////////////////////////
void _(deserialize)(CharStream *stream)
{
  Map *json = JSON_dictionary(stream);

  if (JSON_skipws(stream) == EOF && json != NULL) {
    JSON_free(this);
    memcpy(BASE(0), json, sizeof(Map));
    tfree(json);
  } else {
    JSON_except("Garbage left after object was fully parsed.");
    DELETE (json);
  }
}

/******************************************************************************/
void *STATIC (write)(void *object, CharStream *stream, int indent)
{
  Type *type = gettype(object);

  if (sametype(type, &OBJECT_TYPE(Map))) {
    JSON_write_dictionary(object, stream, indent);
  } else if (sametype(type, &OBJECT_TYPE(Array))) {
    JSON_write_list(object, stream, indent);
  } else if (sametype(type, &OBJECT_TYPE(String))) {
    JSON_write_text(object, stream);
  } else if (sametype(type, &OBJECT_TYPE(double))) {
    JSON_write_number(object, stream);
  }
}

/******************************************************************************/
void STATIC (write_dictionary)(Map *object, CharStream *stream, int indent)
{
  CharStream_putline(stream, "{");

  for (int i = 0; i < object->base.base.size; i++) {
    Pair *current = Array_at(&object->base.base, i);

    JSON_indent(stream, indent + 1);
    CharStream_put(stream, '"');
    JSON_write_text(&current->first.object, stream);
    CharStream_putstr(stream, " : ");
    JSON_write(&current->second.object, stream, indent + 1);

    if (i < object->base.base.size - 1) {
      CharStream_put(stream, ',');
    }

    CharStream_putline(stream, "");
  }

  JSON_indent(stream, indent);
  CharStream_put(stream, '}');
}

/******************************************************************************/
void STATIC (write_list)(Array *object, CharStream *stream, int indent)
{
  CharStream_putline(stream, "[");

  for (int i = 0; i < object->size; i++) {
    void *ptr = Array_atptr(object, i);

    JSON_indent(stream, indent + 1);
    JSON_write(ptr, stream, indent + 1);

    if (i < object->size - 1) {
      CharStream_put(stream, ',');
    }

    CharStream_putline(stream, "");
  }

  JSON_indent(stream, indent);
  CharStream_put(stream, ']');
}

/******************************************************************************/
void STATIC (write_text)(String *object, CharStream *stream)
{
  CharStream_put(stream, '"');
  CharStream_putstr(stream, object->base);
  CharStream_put(stream, '"');
}

/******************************************************************************/
void STATIC (write_number)(double *object, CharStream *stream)
{
  char buffer[256];

  sprintf(buffer, "%f", *object);

  CharStream_putstr(stream, buffer);
}

////////////////////////////////////////////////////////////////////////////////
void _(serialize)(CharStream *stream)
{
  JSON_write(this, stream, 0);
  CharStream_putline(stream, "");
}