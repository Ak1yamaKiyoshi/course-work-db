#include "product_preview.h"
#pragma once

static void product_preview(
    struct mg_connection *connection,
    struct mg_http_message *hm,
    sqlite3 *db) {

  char *type, *buffer, *str_limit, *str_offset;
  int rc = 0, limit = 0, offset = 0;

  type = safe_strdup(get_query_param(hm, "type"));
  str_limit = safe_strdup(get_query_param(hm, "limit"));
  str_offset = safe_strdup(get_query_param(hm, "offset"));

  if (type == NULL)
    return mg_http_reply(connection, HTTP_STATUS_BAD_REQUEST, "", "Provide type as query param");

  if (str_limit != NULL)
    limit = atoi(str_limit);
  
  if (str_offset != NULL)
    offset = atoi(str_offset);

  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(db, API_SQL_PIZZA_PREVIEW, -1, &stmt, NULL);
  sqlite3_bind_text(stmt, 1, type, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 2, limit);
  sqlite3_bind_int(stmt, 3, offset);

  free(type);
  free(str_limit);
  free(str_offset);

  char *results_json = (char *)malloc(sizeof(char) * 1024);
  
  if (results_json == NULL)
    return mg_http_reply(connection, HTTP_STATUS_INTERNAL_SERVER_ERROR, "", "Internal Server Error");
  
  results_json[0] = '\0';

  struct result callback_data = {
    .ptr = 0,
    .capacity = 10,
    .array = malloc(10 * sizeof(struct product))};

  if (callback_data.array == NULL)
    return mg_http_reply(connection, HTTP_STATUS_INTERNAL_SERVER_ERROR, "", "Internal Server Error");    

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    if (callback_data.ptr + 1 >= callback_data.capacity) {
      size_t new_capacity = callback_data.capacity * 2;
      struct product *new_array = realloc(callback_data.array, new_capacity * sizeof(struct product));

      if (new_array == NULL) break;
      callback_data.array = new_array;
      callback_data.capacity = new_capacity;
    }

    size_t i = callback_data.ptr++;
    callback_data.array[i].id = (size_t)(int)sqlite3_column_int(stmt, 0);

    callback_data.array[i].name = safe_strdup((const char *)sqlite3_column_text(stmt, 1));
    callback_data.array[i].price = sqlite3_column_double(stmt, 2);
    callback_data.array[i].type = safe_strdup((const char *)sqlite3_column_text(stmt, 3));
    callback_data.array[i].image = safe_strdup((const char *)sqlite3_column_blob(stmt, 4));
  }

  if (rc != SQLITE_DONE) {
    return mg_http_reply(connection, HTTP_STATUS_INTERNAL_SERVER_ERROR, "", "Internal Server Error. [DB] ");
  }

  sqlite3_finalize(stmt);

  append_str(&results_json, "[");
  for (size_t i = 0; i < callback_data.ptr; i++) {
    struct product row = callback_data.array[i];
    size_t name_len = row.name ? strlen(row.name) : 0;
    size_t price_len = count_double_strlen(row.price, 6);

    // {"":""} - 7 characters
    size_t buffer_size = 14 + 150 + name_len + price_len;
    buffer = (char *)malloc(sizeof(char) * buffer_size);
    
    if (buffer == NULL) continue;

    if (i == callback_data.ptr - 1)
      sprintf(buffer, "{\"name\":\"%s\",\"price\":%lf}", row.name, row.price);
    else
      sprintf(buffer, "{\"name\":\"%s\",\"price\":%lf},", row.name, row.price);
    
    append_str(&results_json, buffer);
    free(buffer);
  }
  
  append_str(&results_json, "]");

  mg_http_reply(connection, HTTP_STATUS_OK, "Content-Type: application/json\r\n", "%s", results_json);

  for (size_t i = 0; i < callback_data.ptr; i++) {
    free(callback_data.array[i].name);
    free(callback_data.array[i].type);
  }

  free(callback_data.array);
  free(results_json);
}
