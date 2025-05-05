#include "list_categories.h"
#pragma once 


static void list_categories(
    struct mg_connection *connection,
    struct mg_http_message *hm, 
    sqlite3 *db
) {
    int rc;

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, API_SQL_LIST_CATEGORIES, -1, &stmt, NULL);
    
    char *results_json = (char*)malloc(sizeof(char) * 1024);
    if (results_json == NULL) {
        return mg_http_reply(connection, HTTP_STATUS_INTERNAL_SERVER_ERROR, "", "Internal Server Error");
    }

    results_json[0] = '\0';
    append_str(&results_json, "[");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        append_str(&results_json, "\"");
        append_str(&results_json, (const char *)sqlite3_column_text(stmt, 0));
        append_str(&results_json, "\",");
    }
    size_t len = strlen(results_json) - 1;
    results_json[len] = ']';
    results_json[len+1] = '\0';
 
    if (rc != SQLITE_DONE) {
        free(results_json);
        return mg_http_reply(connection, HTTP_STATUS_INTERNAL_SERVER_ERROR, "", "Internal Server Error. [DB] ");
    }
    sqlite3_finalize(stmt);
    
    mg_http_reply(connection, HTTP_STATUS_OK, "Content-Type: application/json\r\n", "%s", results_json);
    free(results_json);
}