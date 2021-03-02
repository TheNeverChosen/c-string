#include<stdlib.h>
#include<string.h>
#include "cstring.h"

unsigned short string_valid(const String str){
  return (str!=NULL) ? 1 : 0;
}

StringData* string_data(const String str){
  return string_valid(str) ? ((StringData*)(str))-1 : NULL;
}

StringData* string_ptr_data(const String *str){
  return str ? string_data(*str) : NULL;
}

String string_new_cfg(unsigned short auto_shrink){
  StringData *sd = (StringData*) malloc(sizeof(StringData) + sizeof(char));
  String str = (String) (sd+1);

  sd->sz = sd->cap = 0; sd->auto_shrink = auto_shrink ? 1 : 0;
  (str)[0] = '\0';

  return str;
}

String* string_ptr_new_cfg(unsigned short auto_shrink){
  String *pstr = (String*) malloc(sizeof(String*));
  (*pstr) = string_new_cfg(auto_shrink);
  return pstr;
}

char* string_begin(const String str){
  return (str);
}

char* string_end(const String str){
  return string_valid(str) ? str + (string_data(str))->sz : NULL;
}

size_t string_size(const String str){
  StringData *sd = string_data(str);
  return sd ? sd->sz : (size_t)0;
}

size_t string_size_by_type(const char *str, const StrType sType){
  if(str==NULL) return 0;
  switch(sType){
    case CSTRING_STRING_TYPE: return string_size((String) str);
    case CSTRING_ARR_TYPE: return strlen(str);
    default: return 0;
  }
}

size_t string_capacity(const String str){
  StringData *sd = string_data(str);
  return sd ? sd->cap : (size_t)0;
}

unsigned short string_empty(const String str){
  return (string_size(str)==0);
}

String string_set_size(String *str, const size_t new_size){
  StringData *sd = string_ptr_data(str);

  if(sd==NULL || new_size>=string_npos) return NULL;
  
  if(new_size!=sd->sz){
    if(new_size > sd->cap){
      size_t new_cap = MAX(sd->cap, 1);
      while(new_size > new_cap) new_cap *= _CSTRING_GROWTH_FACTOR;
      sd = ((StringData*) string_set_capacity(str, new_cap)) - 1;
    }
    (*str)[sd->sz] = ' '; (*str)[sd->sz = new_size] = '\0';
    if(new_size < sd->cap && sd->auto_shrink)
      sd = ((StringData*) string_shrink_to_fit(str)) - 1;
  }

  return (*str);
}

String string_set_capacity(String *str, const size_t new_cap){
  StringData *sd = string_ptr_data(str);

  if(sd==NULL || new_cap>=string_npos) return NULL;

  if(new_cap!=sd->cap){
    sd = realloc(sd, sizeof(StringData) + sizeof(char) * (new_cap + 1));
    (*str) = (String) (sd+1);

    if(sd->cap < new_cap) (*str)[sd->cap] = ' ';
    (*str)[sd->cap = new_cap] = '\0';
    sd->sz = MIN(sd->cap, sd->sz);
  }

  return (*str);
}

void string_set_auto_shrink(String *str, const unsigned short auto_shrink){
  StringData *sd = string_ptr_data(str);
  if(sd && (sd->auto_shrink = auto_shrink ? 1 : 0))
    string_shrink_to_fit(str);
}

String string_shrink_to_fit(String *str){
  StringData *sd = string_ptr_data(str);
  size_t new_cap;

  if(sd==NULL) return NULL;
  
  new_cap = sd->cap;
  while(sd->sz < new_cap) new_cap /= _CSTRING_GROWTH_FACTOR;
  return string_set_capacity(str, new_cap*_CSTRING_GROWTH_FACTOR);
}

char string_at(const String str, const size_t index){
  return index < string_size(str) ? str[index] : '\0';
}

char string_front(const String str){
  return string_empty(str) ? '\0' : str[0];
}

char string_back(const String str){
  size_t sz;
  return (sz = string_size(str))==0 ? '\0' : str[sz-1];
}

char* string_set_gap(String *str, const size_t index, const size_t length){
  StringData *sd = string_ptr_data(str);
  size_t to_move;
  char *begin, *end;

  if(sd==NULL || index>sd->sz) return NULL;

  to_move = (sd->sz - index) * sizeof(char);
  string_set_size(str, sd->sz+length);
  begin = (*str) + index; end = begin + length;
  
  memmove(end, begin, to_move);

  return begin;
}

String string_n_insert(String *s1, const size_t index, const char *s2, const size_t length){
  StringData *sd = s1 ? string_data(*s1) : NULL;
  char *temp, *dest;

  if(sd==NULL || index>sd->sz || s2==NULL) return NULL;

  temp = malloc(sizeof(char)*(length+1)); strncpy(temp, s2, length);
  dest = string_set_gap(s1, index, length); strncpy(dest, temp, length); 
  
  free(temp);

  return (*s1);
}

String string_insert(String *s1, const size_t index, const char *s2, const StrType s2Type){
  return string_n_insert(s1, index, s2, string_size_by_type(s2, s2Type));
}

String string_n_append(String *s1, const char *s2, const size_t length){
  return s1 ? string_n_insert(s1, string_size(*s1), s2, length) : NULL;
}

String string_append(String *s1, const char *s2, const StrType s2Type){
  return string_n_append(s1, s2, string_size_by_type(s2, s2Type));
}

String string_insert_c(String *str, const size_t index, const char c){
  char *dest;
  if((dest = string_set_gap(str, index, 1))==NULL) return NULL;
  (*dest) = c;
  return (*str);
}

String string_append_c(String *str, const char c){
  return str ? string_insert_c(str, string_size(*str), c) : NULL;
}

String string_n_erase(String *str, const size_t index, size_t length){
  StringData *sd = string_ptr_data(str);
  char *begin, *end;

  if(sd==NULL || index>=sd->sz) return NULL;

  length = MIN(length, sd->sz-index);
  begin = (*str) + index; end = begin + length;

  memmove(begin, end, (sd->sz-index-length) * sizeof(char));
  string_set_size(str, sd->sz-length);

  return (*str);
}

String string_erase(String *str, const size_t index){
  return string_n_erase(str, index, 1);
}

String string_clear(String *str){
  return string_set_size(str, 0);
}

void string_free(String str){
  free(string_data(str));
}

void string_ptr_free(String *str){
  free(string_ptr_data(str));
  free(str);
}