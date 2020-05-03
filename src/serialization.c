#include "serialization.h"

#include <assert.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>

#define FMT_BUF_SIZE 64

void serialize_version(FILE *file, const char *name, const char *ver) {
  fprintf(file, "%s=%s\n", name, ver);
}

void serialize_tag(FILE *file, const char *name) {
  fprintf(file, "#%s\n", name);
}

void serialize_ushort(FILE *file, const char *name, unsigned short value) {
  fprintf(file, "%s=%hu\n", name, value);
}

void serialize_int(FILE *file, const char *name, int value) {
  fprintf(file, "%s=%d\n", name, value);
}

void serialize_uint(FILE *file, const char *name, unsigned int value) {
  fprintf(file, "%s=%u\n", name, value);
}

void serialize_ulong(FILE *file, const char *name, unsigned long value) {
  fprintf(file, "%s=%lu\n", name, value);
}

void serialize_string(FILE *file, const char *name, const char *value) {
  if (value == NULL) {
    fprintf(file, "%s=-1|\n", name);
  } else {
    fprintf(file, "%s=%d|%s\n", name, (int)strlen(value), value);
  }
}

void serialize_ushort_tab(
  FILE *file, const char *name, const unsigned short *data, size_t size)
{
  fprintf(file, "%s=", name);
  for (size_t i = 0; i < size; ++i) {
    fprintf(file, " %hu", data[i]);
  }
  fprintf(file, "\n");
}

void serialize_ulong_tab(
  FILE *file, const char *name, const unsigned long *data, size_t size)
{
  fprintf(file, "%s=", name);
  for (size_t i = 0; i < size; ++i) {
    fprintf(file, " %lu", data[i]);
  }
  fprintf(file, "\n");
}

void deserialize_version(FILE *file, const char *name, const char *ver) {
  assert(strlen(name) < FMT_BUF_SIZE - 16);
  char fmt_buf[FMT_BUF_SIZE];
  sprintf(fmt_buf, " %s = %%60s", name);
  char ver_buf[64];
  if (fscanf(file, fmt_buf, ver_buf) != 1) {
    error(EXIT_FAILURE, 0, "invalid world file (bad version)");
  }
  if (strcmp(ver_buf, ver) != 0) {
    error(EXIT_FAILURE, 0,
      "world file was created by a different version of the program");
  }
}

void deserialize_tag(FILE *file, const char *name) {
  char name_buf[64];
  if (fscanf(file, " #%60s", name_buf) != 1
    || strcmp(name_buf, name) != 0)
  {
    error(EXIT_FAILURE, 0, "invalid world file (at tag %s)", name);
  }
}

void deserialize_ushort(
  FILE *file, const char *name, unsigned short *value,
  unsigned short min, unsigned short max)
{
  assert(strlen(name) < FMT_BUF_SIZE - 16);
  char fmt_buf[FMT_BUF_SIZE];
  sprintf(fmt_buf, " %s = %%hu", name);
  if (fscanf(file, fmt_buf, value) != 1 || *value < min || *value > max) {
    error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
  }
}

void deserialize_int(
  FILE *file, const char *name, int *value, int min, int max)
{
  assert(strlen(name) < FMT_BUF_SIZE - 16);
  char fmt_buf[FMT_BUF_SIZE];
  sprintf(fmt_buf, " %s = %%d", name);
  if (fscanf(file, fmt_buf, value) != 1 || *value < min || *value > max) {
    error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
  }
}

void deserialize_uint(
  FILE *file, const char *name, unsigned int *value,
  unsigned int min, unsigned int max)
{
  assert(strlen(name) < FMT_BUF_SIZE - 16);
  char fmt_buf[FMT_BUF_SIZE];
  sprintf(fmt_buf, " %s = %%u", name);
  if (fscanf(file, fmt_buf, value) != 1 || *value < min || *value > max) {
    error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
  }
}

void deserialize_ulong(
  FILE *file, const char *name, unsigned long *value,
  unsigned long min, unsigned long max)
{
  assert(strlen(name) < FMT_BUF_SIZE - 16);
  char fmt_buf[FMT_BUF_SIZE];
  sprintf(fmt_buf, " %s = %%lu", name);
  if (fscanf(file, fmt_buf, value) != 1 || *value < min || *value > max) {
    error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
  }
}

void deserialize_string(FILE *file, const char *name, const char **string) {
  assert(strlen(name) < FMT_BUF_SIZE - 16);
  char fmt_buf[FMT_BUF_SIZE];
  sprintf(fmt_buf, " %s = %%d |%%n", name);
  int len, n;
  n = 0;
  const int MAX_STRING_LENGTH = 4096;
  if (fscanf(file, fmt_buf, &len, &n) != 1
    || len > MAX_STRING_LENGTH
    || n == 0)
  {
    error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
  }
  if (len < 0) {
    *string = NULL;
  } else {
    char *data = malloc(len+1);
    data[len] = 0;
    if (len > 0) {
      char fmt_buf[16];
      sprintf(fmt_buf, "%%%dc%%n", len);
      fscanf(file, fmt_buf, data, &n);
      if (n != len) {
        error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
      }
    }
  }
}

void deserialize_ushort_tab(
  FILE *file, const char *name, unsigned short *data, size_t size,
  unsigned short min, unsigned short max)
{
  assert(strlen(name) < FMT_BUF_SIZE - 16);
  char fmt_buf[FMT_BUF_SIZE];
  sprintf(fmt_buf, " %s = %%n", name);
  int n = 0;
  if (fscanf(file, fmt_buf, &n) != 0 || n == 0) {
    error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
  }
  for (size_t i = 0; i < size; ++i) {
    if (fscanf(file, " %hu", &data[i]) != 1
      || data[i] < min || data[i] > max)
    {
      error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
    }
  }
}

void deserialize_ulong_tab(
  FILE *file, const char *name, unsigned long *data, size_t size,
  unsigned long min, unsigned long max)
{
  assert(strlen(name) < FMT_BUF_SIZE - 16);
  char fmt_buf[FMT_BUF_SIZE];
  sprintf(fmt_buf, " %s = %%n", name);
  int n = 0;
  if (fscanf(file, fmt_buf, &n) != 0 || n == 0) {
    error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
  }
  for (size_t i = 0; i < size; ++i) {
    if (fscanf(file, " %lu", &data[i]) != 1
      || data[i] < min || data[i] > max)
    {
      error(EXIT_FAILURE, 0, "invalid world file (at field %s)", name);
    }
  }
}
