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

#pragma once
#define API_SQL_PIZZA_PREVIEW "SELECT id, name, price, type, image FROM product WHERE type = ? LIMIT ? OFFSET ?"

static void product_preview(
    struct mg_connection *connection,
    struct mg_http_message *hm,
    sqlite3 *db);