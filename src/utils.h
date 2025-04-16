#pragma once

char* safe_strdup(const char* str);
size_t count_int_strlen(size_t n);
size_t count_double_strlen(double value, size_t precision);
void append_str(char **dest, const char *src);
