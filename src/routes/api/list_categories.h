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
#define API_SQL_LIST_CATEGORIES "select type from product group by type"