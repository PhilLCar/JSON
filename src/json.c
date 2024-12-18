#include <json.h>

#define TYPENAME JSON

/******************************************************************************/
char STATIC (skipws)(CharStream *stream)
{
  char c;

  while ((c = CharStream_Peek(stream)) != EOF) {
    switch (c) {
      case '\n':
      case '\r':
      case ' ':
      case '\t':
        CharStream_Get(stream);
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
    CharStream_Put(stream, ' ');
  }
}

////////////////////////////////////////////////////////////////////////////////
JSON *_(Construct)()
{  
  return (JSON*)Map_Construct(BASE(0), TYPEOF (String));
}

////////////////////////////////////////////////////////////////////////////////
void _(Destruct)()
{
  if (this) {
    Map_Destruct(BASE(0));
  }
}

/******************************************************************************/
void *STATIC (any)(CharStream*);

/******************************************************************************/
String *STATIC (text)(CharStream *stream)
{
  String *text = NEW (String) ("");
  char c = JSON_skipws(stream);

  if (c == '"') {
    CharStream_Get(stream);

    while ((c = CharStream_Get(stream)) != EOF) {
      switch (c) {
        case '"':
          break;
        case '\\':
          c = CharStream_Escape(stream);
        default:
          String_Append(text, c);
          continue;
      }
      break;
    }
  } else {
    THROW(NEW (JSONException)("Expecting '\"' at the begining of a string."));
  }

  if (c == EOF) {
    THROW(NEW (JSONException)("Reached EOF before deserialization was over!"));
  }

  return text;
}

/******************************************************************************/
double *STATIC (number)(CharStream *stream)
{
  String *digits = NEW (String) ("");
  double *number = talloc(TYPEOF (double));
  char    c;

  JSON_skipws(stream);

  // For now numbers are going to be decimal floating points only
  while ((c = CharStream_Peek(stream)) != EOF) {
    if (c != ',' && c != ']' && c != '}') {
      String_Append(digits, CharStream_Get(stream));
    } else break;
  }

  if (c == EOF) {
    THROW(NEW (JSONException)("Reached EOF before deserialization was over!"));
  }

  *number = atof(digits->base);

  DELETE (digits);

  return number;
}

/******************************************************************************/
Map *STATIC (map)(CharStream *stream)
{
  Map *map = NEW (Map) (TYPEOF (String));

  char c = JSON_skipws(stream);

  if (c == '{') {
    CharStream_Get(stream);

    while ((c = JSON_skipws(stream)) != EOF) {
      if (c == '}') break;

      String *key = JSON_text(stream);

      if (JSON_skipws(stream) == ':') {
        CharStream_Get(stream);
      } else {
        char error[256];
        sprintf(error, "Expecting ':' after key '%s'!", key->base);
        THROW(NEW(JSONException)(error));
      }

      void *value = JSON_any(stream);

      Map_Set(map, key, value);

      if ((c = JSON_skipws(stream)) == ',') {
        CharStream_Get(stream);
      } else break;
    }

    if (CharStream_Get(stream) != '}') {
      THROW(NEW (Exception)("Expecting '}' at the end of a dictonary (look for a missing ',')."));
    }

  } else {
    THROW(NEW (Exception)("Expecting '{' at the begining of a dictonary."));
  }

  if (c == EOF) {
    THROW(NEW (Exception)("Reached EOF before deserialization was over!"));
  }

  return map;
}

/******************************************************************************/
List *STATIC (list)(CharStream *stream)
{
  List *list = NEW (List) ();

  char c = JSON_skipws(stream);

  if (c == '[') {
    CharStream_Get(stream);

    while ((c = JSON_skipws(stream)) != EOF) {
      if (c == ']') break;

      void *value = JSON_any(stream);

      List_Add(list, value);

      if ((c = JSON_skipws(stream)) == ',') {
        CharStream_Get(stream);
      } else break;
    }

    if (CharStream_Get(stream) != ']') {
      THROW(NEW (Exception)("Expecting ']' at the end of a array (look for a missing ',')."));
    }
  } else {
    THROW(NEW (Exception)("Expecting '[' at the begining of a array."));
  }
  
  if (c == EOF) {
    THROW(NEW (Exception)("Reached EOF before deserialization was over!"));
  }

  return list;
}

/******************************************************************************/
void *STATIC (any)(CharStream *stream)
{
  char c = JSON_skipws(stream);

  switch (c) {
    case '{':
      return JSON_map(stream);
    case '[':
      return JSON_list(stream);
    case '"':
      return JSON_text(stream);
    default:
      return JSON_number(stream);
  }
}

////////////////////////////////////////////////////////////////////////////////
JSON *_(Deserialize)(CharStream *stream)
{
  Map *deserialized = JSON_map(stream);

  JSON_Destruct(this);
  memcpy(this, deserialized, sizeof(Map));

  tfree(deserialized);
  
  return this;
}

/******************************************************************************/
void STATIC (write)(void*, CharStream*, int);

/******************************************************************************/
void STATIC (write_text)(String *object, CharStream *stream)
{
  CharStream_Put(stream, '"');
  CharStream_WriteString(stream, object);
  CharStream_Put(stream, '"');
}

/******************************************************************************/
void STATIC (write_number)(double *object, CharStream *stream)
{
  CharStream_WriteString(stream, object);
}

/******************************************************************************/
void STATIC (write_map)(Map *object, CharStream *stream, int indent)
{
  CharStream_PutLn(stream, "{");

  for (List *l = &object->base; !List_Empty(l); l = List_Next(l)) {
    Pair *current = List_Head(l);

    JSON_indent(stream, indent + 1);
    JSON_write_text(current->first, stream);
    CharStream_PutStr(stream, " : ");
    JSON_write(current->second, stream, indent + 1);

    if (!List_Empty(List_Next(l))) {
      CharStream_Put(stream, ',');
    }

    CharStream_PutLn(stream, "");
  }

  JSON_indent(stream, indent);
  CharStream_Put(stream, '}');
}

/******************************************************************************/
void STATIC (write_list)(List *object, CharStream *stream, int indent)
{
  CharStream_PutLn(stream, "[");

  for (List *l = object; !List_Empty(l); l = List_Next(l)) {
    void *ptr = List_Head(l);

    JSON_indent(stream, indent + 1);
    JSON_write(ptr, stream, indent + 1);

    if (!List_Empty(List_Next(l))) {
      CharStream_Put(stream, ',');
    }

    CharStream_PutLn(stream, "");
  }

  JSON_indent(stream, indent);
  CharStream_Put(stream, ']');
}

/******************************************************************************/
void STATIC (write)(void *object, CharStream *stream, int indent)
{
  const Type *type = gettype(object);

  if (sametype(type, TYPEOF (List))) {
    JSON_write_list(object, stream, indent);
  } else if (sametype(type, TYPEOF (String))) {
    JSON_write_text(object, stream);
  } else if (sametype(type, TYPEOF (double))) {
    JSON_write_number(object, stream);
  } else {
    JSON_write_map(object, stream, indent);
  }
}

////////////////////////////////////////////////////////////////////////////////
void _(Serialize)(CharStream *stream)
{
  JSON_write(this, stream, 0);
  CharStream_PutLn(stream, "");
}

#undef TYPENAME

#define TYPENAME JSONException

JSONException *_(Construct)(const char *message)
{
  return (JSONException*)Exception_Construct(BASE(0), message);
}

void _(Destruct)()
{
  Exception_Destruct(BASE(0));
}

#undef TYPENAME