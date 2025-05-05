#include "product_create.h"

static void product_create(
    struct mg_connection *connection,
    struct mg_http_message *hm, 
    sqlite3 *db
) {
    int rc;

    char name[100] = "";
    char description[500] = "";
    char price[20] = "";
    char type[50] = "";
    char size[20] = "";
    char is_custom[2] = "";
    void *image_data = NULL;
    size_t image_size = 0;

    struct mg_http_part part;

    size_t offset = 0;
    while ((offset = mg_http_next_multipart(hm->body, offset, &part)) > 0) {
        if (mg_strcmp(part.name, mg_str("name")) == 0) {
            mg_snprintf(name, sizeof(name), "%.*s", (int) part.body.len, part.body.buf);        
        } else if (mg_strcmp(part.name, mg_str("description")) == 0) {
            mg_snprintf(description, sizeof(description), "%.*s", (int) part.body.len, part.body.buf);
        } else if (mg_strcmp(part.name, mg_str("price")) == 0) {
            mg_snprintf(price, sizeof(price), "%.*s", (int) part.body.len, part.body.buf);
        } else if (mg_strcmp(part.name, mg_str("type")) == 0) {
            mg_snprintf(type, sizeof(type), "%.*s", (int) part.body.len, part.body.buf);
        } else if (mg_strcmp(part.name, mg_str("size")) == 0) {
            mg_snprintf(size, sizeof(size), "%.*s", (int) part.body.len, part.body.buf);
        } else if (mg_strcmp(part.name, mg_str("is_custom")) == 0) {
            mg_snprintf(is_custom, sizeof(is_custom), "%.*s", (int) part.body.len, part.body.buf);
        } else if (mg_strcmp(part.name, mg_str("image")) == 0) {
            // Copy image data
            image_data = malloc(part.body.len);
            if (image_data != NULL) {
                memcpy(image_data, part.body.buf, part.body.len);
                image_size = part.body.len;
            }
        }
    }

    // printf("Received form data:\n");
    // printf("name: %s\n", name);
    // printf("description: %s\n", description);
    // printf("price: %s\n", price);
    // printf("type: %s\n", type);
    // printf("size: %s\n", size);
    // printf("is_custom: %s\n", is_custom);
    // printf("image size: %zu bytes\n", image_size);
    // fflush(stdout);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, API_SQL_CREATE_PRODUCT, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, description, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, price, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, type, -1, SQLITE_STATIC);
        
        if (image_data != NULL && image_size > 0) {
            sqlite3_bind_blob(stmt, 5, image_data, image_size, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 5);
        }
        
        sqlite3_bind_text(stmt, 6, size, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 7, is_custom, -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_DONE) {
            printf("Product inserted successfully\n");
            mg_http_reply(connection, 200, "Content-Type: application/json\r\n", 
                "{\"status\":\"success\",\"message\":\"Product created\"}\n");
        } else {
            printf("Error inserting product: %s\n", sqlite3_errmsg(db));
            mg_http_reply(connection, 500, "Content-Type: application/json\r\n", 
                "{\"status\":\"error\",\"message\":\"Database error\"}\n");
        }
        
        sqlite3_finalize(stmt);

    } else {
        printf("SQL preparation error: %s\n", sqlite3_errmsg(db));
        mg_http_reply(connection, 500, "Content-Type: application/json\r\n", 
            "{\"status\":\"error\",\"message\":\"SQL preparation error\"}\n");
    }

    if (image_data != NULL) {
        free(image_data);
    }
}