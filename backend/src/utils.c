#pragma once
#include "utils.h"

void append_str(char **dest, const char *src) {
  if (*dest == NULL) {
    *dest = strdup(src);
    return;
  }

  size_t dest_len = strlen(*dest);
  size_t src_len = strlen(src);
  size_t new_len = dest_len + src_len + 1;

  char *new_str = realloc(*dest, new_len);
  if (new_str == NULL) {
    return;
  }

  strcat(new_str + dest_len, src);
  *dest = new_str;
}

char *safe_strdup(const char *str) {
  return str ? strdup(str) : strdup("");
}

size_t count_int_strlen(size_t n) {
  size_t count = 0;
  do {
    n /= 10;
    ++count;
  } while (n != 0);
  return count;
}

size_t count_double_strlen(double value, size_t precision) {
  size_t count = 0;
  if (value < 0) {
    count = 1;
    value = -value;
  }

  size_t int_part = (size_t)value;
  count += (int_part == 0) ? 1 : (size_t)(int)((double)log10((double)int_part) + 1);
  count += 1;
  count += precision;
  return count;
}

static char *get_query_param(struct mg_http_message *hm, const char *name) {
  char *result = malloc(256);
  if (result == NULL) return NULL;
  
  result[0] = '\0';

  struct mg_str query = mg_str_n(hm->uri.buf + hm->uri.len, 0);
  if (hm->query.len > 0) {
    query = hm->query;
  }

  if (mg_http_get_var(&query, name, result, 256) > 0) {
    return result;
  }

  free(result);
  return NULL;
}
