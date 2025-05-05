#include "mongoose/mongoose.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "include/types/http.c"
#include "include/types/db.c"
#include "src/utils.c"


#pragma once
#define API_SQL_CREATE_PRODUCT "INSERT INTO product (name, description, price, type, image, size, is_custom) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
