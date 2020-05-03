#ifndef __SERIALIZATION_H
#define __SERIALIZATION_H

#include <stdio.h>

void serialize_version(FILE *file, const char *name, const char *ver);
void serialize_tag(FILE *file, const char *name);
void serialize_ushort(FILE *file, const char *name, unsigned short value);
void serialize_int(FILE *file, const char *name, int value);
void serialize_uint(FILE *file, const char *name, unsigned int value);
void serialize_ulong(FILE *file, const char *name, unsigned long value);
void serialize_string(FILE *file, const char *name, const char *value);
void serialize_ushort_tab(
  FILE *file, const char *name, const unsigned short *data, size_t size);
void serialize_ulong_tab(
  FILE *file, const char *name, const unsigned long *data, size_t size);

void deserialize_version(FILE *file, const char *name, const char *ver);
void deserialize_tag(FILE *file, const char *name);
void deserialize_ushort(
  FILE *file, const char *name, unsigned short *value,
  unsigned short min, unsigned short max);
void deserialize_int(
  FILE *file, const char *name, int *value, int min, int max);
void deserialize_uint(
  FILE *file, const char *name, unsigned int *value,
  unsigned int min, unsigned int max);
void deserialize_ulong(
  FILE *file, const char *name, unsigned long *value,
  unsigned long min, unsigned long max);
void deserialize_string(FILE *file, const char *name, const char **string);
void deserialize_ushort_tab(
  FILE *file, const char *name, unsigned short *data, size_t size,
  unsigned short min, unsigned short max);
void deserialize_ulong_tab(
  FILE *file, const char *name, unsigned long *data, size_t size,
  unsigned long min, unsigned long max);

#define SERIALIZE_USHORT(file,obj,fld) \
  serialize_ushort(file, #fld, (obj)->fld)
#define SERIALIZE_INT(file,obj,fld) \
  serialize_int(file, #fld, (obj)->fld)
#define SERIALIZE_UINT(file,obj,fld) \
  serialize_uint(file, #fld, (obj)->fld)
#define SERIALIZE_ULONG(file,obj,fld) \
  serialize_ulong(file, #fld, (obj)->fld)
#define SERIALIZE_STRING(file,obj,fld) \
  serialize_string(file, #fld, (obj)->fld)

#define SERIALIZE_USHORT_TAB(file,obj,fld,size) \
  serialize_ushort_tab(file, #fld, (obj)->fld, (size))
#define SERIALIZE_ULONG_TAB(file,obj,fld,size) \
  serialize_ulong_tab(file, #fld, (obj)->fld, (size))

#define DESERIALIZE_USHORT(file,obj,fld,min,max) \
  deserialize_ushort(file, #fld, &(obj)->fld, (min), (max))
#define DESERIALIZE_INT(file,obj,fld,min,max) \
  deserialize_int(file, #fld, &(obj)->fld, (min), (max))
#define DESERIALIZE_UINT(file,obj,fld,min,max) \
  deserialize_uint(file, #fld, &(obj)->fld, (min), (max))
#define DESERIALIZE_ULONG(file,obj,fld,min,max) \
  deserialize_ulong(file, #fld, &(obj)->fld, (min), (max))
#define DESERIALIZE_STRING(file,obj,fld) \
  deserialize_string(file, #fld, &(obj)->fld)

#define DESERIALIZE_USHORT_TAB(file,obj,fld,size,min,max) \
  deserialize_ushort_tab(file, #fld, (obj)->fld, (size), (min), (max))
#define DESERIALIZE_ULONG_TAB(file,obj,fld,size,min,max) \
  deserialize_ulong_tab(file, #fld, (obj)->fld, (size), (min), (max))

#endif
