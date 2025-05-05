#include "mongoose/mongoose.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "include/types/http.c"
#include "include/types/db.c"
#include "src/utils.c"
#include "src/routes/api/product_preview.c"
#include "src/routes/api/list_categories.c"
#include "src/routes/api/product_create.c"

static void request_handler(struct mg_connection *connection, int event, void *event_data);

static void request_handler(struct mg_connection *connection, int event, void *event_data) {
  if (event != MG_EV_HTTP_MSG)
    return;

    
  sqlite3 *db = connection->fn_data;

  struct mg_http_message *hm = (struct mg_http_message *)event_data;
  if (mg_match(hm->uri, mg_str("/api/products"), NULL)) {

    product_preview(connection, hm, db);
  } else if ( mg_match(hm->uri, mg_str("/api/categories"), NULL)) {
    list_categories(connection, hm, db); 
  } else if (mg_match(hm->uri, mg_str("/api/product/create"), NULL)) {
    product_create(connection, hm, db); 
  }
  else {
    mg_http_reply(connection, HTTP_STATUS_NOT_FOUND, "", "Not found\n");
  }
}

int main()
{
  struct mg_mgr connection_manager;
  mg_mgr_init(&connection_manager);

  sqlite3 *db;
  sqlite3_open("pizerria.db", &db);
  sqlite3_exec(db, "PRAGMA synchronous = OFF;", 0, 0, 0);
  sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", 0, 0, 0);
  sqlite3_exec(db, "PRAGMA temp_store = MEMORY;", 0, 0, 0);


  mg_http_listen(&connection_manager, "http://0.0.0.0:8000", request_handler, db);
  for (;;) {
    mg_mgr_poll(&connection_manager, 1000);
  }

  return 0;
}

