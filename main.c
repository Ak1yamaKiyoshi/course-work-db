#include "mongoose/mongoose.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "include/types/db.c"


char* safe_strdup(const char* str);
size_t count_int_strlen(size_t n);
size_t count_double_strlen(double value, size_t precision);
void append_str(char **dest, const char *src);
static void request_handler(struct mg_connection *connection, int event, void *event_data);


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


char* safe_strdup(const char* str) {
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


static char* get_query_param(struct mg_http_message *hm, const char *name) {
  static char result[256];
  result[0] = '\0';

  struct mg_str query = mg_str_n(hm->uri.buf + hm->uri.len, 0);
  if (hm->query.len > 0) {
    query = hm->query;
  }

  if (mg_http_get_var(&query, name, result, sizeof(result)) > 0) {
    return result;
  }

  return NULL;
}


static void request_handler(struct mg_connection *connection, int event, void *event_data) { 
  if (event != MG_EV_HTTP_MSG) 
    return;

  sqlite3 *db = connection->fn_data;
  char *err_msg = 0;
  int rc;
  char *type; 
  char *buffer;

  struct mg_http_message *hm = (struct mg_http_message *) event_data; 
  if (mg_match(hm->uri, mg_str("/api/pizzas"), NULL)) { 

    type = get_query_param(hm, "type");
    if (type == NULL) {
      mg_http_reply(connection, 400, "", "Provide type");
    }

    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, name, price, type FROM product WHERE type = ?";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
      mg_http_reply(connection, 400, "", "Invalid type. ");
      fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
      free(type);
      return;
    }

    rc = sqlite3_bind_text(stmt, 1, type, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
      fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
      sqlite3_finalize(stmt);
      free(type);
      return;
    }

    char *results_json = (char*)malloc(sizeof(char) * 1024);
    results_json[0] = '\0';
    if (results_json == NULL) {
      fprintf(stderr, "Memory allocation failed for results_json\n");
      mg_http_reply(connection, 500, "", "Internal Server Error");
      free(type);
      return;
    }

    struct result callback_data = {
      .ptr = 0,
      .capacity = 10,
      .array = malloc(10 * sizeof(struct product))
    };

    if (callback_data.array == NULL) {
      fprintf(stderr, "Memory allocation failed for results_json\n");
      mg_http_reply(connection, 500, "", "Internal Server Error");
      free(type);
      return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      if (callback_data.ptr + 1 >= callback_data.capacity) {
          size_t new_capacity = callback_data.capacity * 2;
          struct product *new_array = realloc(callback_data.array, 
                                             new_capacity * sizeof(struct product));
          if (new_array == NULL) {
              fprintf(stderr, "Memory reallocation failed\n");
              break;
          }
          callback_data.array = new_array;
          callback_data.capacity = new_capacity;
      }
      
      size_t i = callback_data.ptr++;
      callback_data.array[i].id = sqlite3_column_int(stmt, 0);
      
      callback_data.array[i].name = safe_strdup((const char*)sqlite3_column_text(stmt, 1));
      callback_data.array[i].price = sqlite3_column_double(stmt, 2);
      callback_data.array[i].type = safe_strdup((const char*)sqlite3_column_text(stmt, 3));
    }
   
    if (rc != SQLITE_DONE) {
      fprintf(stderr, "Error executing statement: %s\n", sqlite3_errmsg(db));
    }
  
    sqlite3_finalize(stmt);
   

    append_str(&results_json, "[");
    for (size_t i = 0; i < callback_data.ptr; i++) {
      struct product row = callback_data.array[i];
      size_t name_len = row.name ? strlen(row.name) : 0;
      size_t price_len = count_double_strlen(row.price, 6);
      // {"":""} - 7 characters
      size_t buffer_size = 14 + 150 + name_len + price_len;
      buffer = (char*)malloc(sizeof(char) * buffer_size);
      if (buffer == NULL) 
        continue;

      if ( i == callback_data.ptr - 1)
        sprintf(buffer, "{\"name\":\"%s\",\"price\":%lf}", row.name, row.price);
      else 
        sprintf(buffer, "{\"name\":\"%s\",\"price\":%lf},", row.name, row.price);
        append_str(&results_json, buffer);
      free(buffer);
    }
    append_str(&results_json, "]");

    mg_http_reply(connection, 200, "Content-Type: application/json\r\n", "%s", results_json);

    for (size_t i = 0; i < callback_data.ptr; i++) {
      free(callback_data.array[i].name);
      free(callback_data.array[i].type); 
    }
    
    free(callback_data.array);
    free(results_json);


  } else {
    mg_http_reply(connection, 404, "", "Not found\n");
  }
}


int main() {
  struct mg_mgr connection_manager;
  mg_mgr_init(&connection_manager);

  sqlite3 *db;
  
  char *err_msg = 0;

  int rc;
  rc = sqlite3_open("pizerria.db", &db);
  sqlite3_exec(db, "PRAGMA synchronous = OFF;", 0, 0, 0);
  sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", 0, 0, 0);
  sqlite3_exec(db, "PRAGMA temp_store = MEMORY;", 0, 0, 0);


  if (rc != SQLITE_OK) {
      fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
  }

  mg_http_listen(&connection_manager, "http://0.0.0.0:8000", request_handler, db);
  for (;;) {
    mg_mgr_poll(&connection_manager, 1000);
  }
  return 0;
}
