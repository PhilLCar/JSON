#include <json.h>

#define TYPENAME JSON

OBJECTIFY(double);

/******************************************************************************/
void STATIC (except) (const char *message)
{
  fprintf(stderr, "%s\n", message);
  exit(-1);
}

/******************************************************************************/
char STATIC (skipws)(CharStream *stream)
{
  char c;

  while ((c = CharStream_peek(stream)) != EOF) {
    switch (c) {
      case '\n':
      case '\r':
      case ' ':
      case '\t':
        CharStream_get(stream);
        continue;
      default:
        break;
    }
    break;
  }

  return c;
}

/******************************************************************************/
void STATIC (indent)(CharStream *stream, int indent)
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

/******************************************************************************/
void STATIC (free_any)(void*);

/******************************************************************************/
void STATIC (free_map)(Map *object)
{
  for (int i = 0; i < object->base.base.size; i++) {
    Pair *current = Array_at(&object->base.base, i);

    JSON_free_any(*(void**)current->second.object);
  }
}

/******************************************************************************/
void STATIC (free_array)(Array *object)
{
  for (int i = 0; i < object->size; i++) {
    void *ptr = Array_atptr(object, i);

    JSON_free_any(ptr);
  }
}

/******************************************************************************/
void STATIC (free_any)(void *object)
{
  const Type *type = gettype(object);

  if (sametype(type, &OBJECT_TYPE(Array))) {
    JSON_free_array((Array*)object);
  } else if (!sametype(type, &OBJECT_TYPE(String)) && !sametype(type, &OBJECT_TYPE(double))) {
    JSON_free_map((Map*)object);
  }

  DELETE(object);
}

////////////////////////////////////////////////////////////////////////////////
void _(free)()
{
  JSON_free_map(BASE(0));
  Map_free(BASE(0));
}

/******************************************************************************/
void *STATIC (any)(CharStream*);

/******************************************************************************/
String *STATIC (text)(CharStream *stream)
{
  String *text = NEW (String) ("");
  char c = JSON_skipws(stream);

  if (c == '"') {
    CharStream_get(stream);

    while ((c = CharStream_read(stream)) != EOF) {
      switch (c) {
        case '"':
          break;
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
  while ((c = CharStream_peek(stream)) != EOF) {
    if ((c >= '0' && c <= '9') || c == '.') {
      String_append(digits, CharStream_read(stream));
    } else break;
  }

  if (c == EOF) {
    JSON_except("Reached EOF before deserialization was over!");
  }

  *number = atof(digits->base);

  DELETE (digits);

  return number;
}

/******************************************************************************/
Map *STATIC (map)(CharStream *stream)
{
  Map *map = NEW (Map) (OBJECT_TYPE(String), NATIVE_TYPE(void*), (Comparer)String_cequals);

  char c = JSON_skipws(stream);

  if (c == '{') {
    CharStream_get(stream);

    while ((c = JSON_skipws(stream)) != EOF) {
      if (c == '}') break;

      String *key = JSON_text(stream);

      if (JSON_skipws(stream) == ':') {
        CharStream_get(stream);
      } else {
        char error[256];
        sprintf(error, "Expecting ':' after key '%s'!", key->base);
        JSON_except(error);
      }

      void *value = JSON_any(stream);

      Map_setkey(map, key, &value);

      if ((c = JSON_skipws(stream)) == ',') {
        CharStream_get(stream);
      } else break;
    }

    if (CharStream_get(stream) != '}') {
      JSON_except("Expecting '}' at the end of a dictonary (look for a missing ',').");
    }

  } else {
    JSON_except("Expecting '{' at the begining of a dictonary.");
  }

  if (c == EOF) {
    JSON_except("Reached EOF before deserialization was over!");
  }

  return map;
}

/******************************************************************************/
Array *STATIC (array)(CharStream *stream)
{
  Array *array = NEW (Array) (sizeof(void*));

  char c = JSON_skipws(stream);

  if (c == '[') {
    CharStream_get(stream);

    while ((c = JSON_skipws(stream)) != EOF) {
      if (c == ']') break;

      void *value = JSON_any(stream);

      Array_push(array, &value);

      if ((c = JSON_skipws(stream)) == ',') {
        CharStream_get(stream);
      } else break;
    }

    if (CharStream_get(stream) != ']') {
      JSON_except("Expecting ']' at the end of a array (look for a missing ',').");
    }
  } else {
    JSON_except("Expecting '[' at the begining of a array.");
  }
  
  if (c == EOF) {
    JSON_except("Reached EOF before deserialization was over!");
  }

  return array;
}

/******************************************************************************/
void *STATIC (any)(CharStream *stream)
{
  char c = JSON_skipws(stream);

  switch (c) {
    case '{':
      return JSON_map(stream);
    case '[':
      return JSON_array(stream);
    case '"':
      return JSON_text(stream);
    default:
      return JSON_number(stream);
  }
}

////////////////////////////////////////////////////////////////////////////////
void _(deserialize)(CharStream *stream)
{
  Map *json = JSON_map(stream);

  if (json != NULL) {
    JSON_free(this);
    memcpy(BASE(0), json, sizeof(Map));
    tfree(json);
  }
}

/******************************************************************************/
void STATIC (write)(void*, CharStream*, int);

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

  sprintf(buffer, "%g", *object);

  CharStream_putstr(stream, buffer);
}

/******************************************************************************/
void STATIC (write_map)(Map *object, CharStream *stream, int indent)
{
  CharStream_putline(stream, "{");

  for (int i = 0; i < object->base.base.size; i++) {
    Pair *current = Array_at(&object->base.base, i);

    JSON_indent(stream, indent + 1);
    JSON_write_text((String*)current->first.object, stream);
    CharStream_putstr(stream, " : ");
    JSON_write(*(void**)current->second.object, stream, indent + 1);

    if (i < object->base.base.size - 1) {
      CharStream_put(stream, ',');
    }

    CharStream_putline(stream, "");
  }

  JSON_indent(stream, indent);
  CharStream_put(stream, '}');
}

/******************************************************************************/
void STATIC (write_array)(Array *object, CharStream *stream, int indent)
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
void STATIC (write)(void *object, CharStream *stream, int indent)
{
  const Type *type = gettype(object);

  if (sametype(type, &OBJECT_TYPE(Array))) {
    JSON_write_array(object, stream, indent);
  } else if (sametype(type, &OBJECT_TYPE(String))) {
    JSON_write_text(object, stream);
  } else if (sametype(type, &OBJECT_TYPE(double))) {
    JSON_write_number(object, stream);
  } else {
    JSON_write_map(object, stream, indent);
  }
}

////////////////////////////////////////////////////////////////////////////////
void _(serialize)(CharStream *stream)
{
  JSON_write(this, stream, 0);
  CharStream_putline(stream, "");
}