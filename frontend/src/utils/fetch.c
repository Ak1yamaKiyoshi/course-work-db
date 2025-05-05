#pragma once
#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef void (*response_callback)(const char* data, size_t length, void* user_data);

typedef struct {
    response_callback callback;
    void* user_data;
    bool completed;
    char* url_copy; 
} request_context;

static void http_callback(struct mg_connection *c, int ev, void *ev_data) {
    request_context* ctx = (request_context*)c->fn_data;
    if (!ctx) return;
    
    if (ev == MG_EV_CONNECT) {
        struct mg_str host = mg_url_host(ctx->url_copy);
        mg_printf(c, "GET %s HTTP/1.1\r\n"
                    "Host: %.*s\r\n"
                    "Connection: close\r\n\r\n", 
                    mg_url_uri(ctx->url_copy),
                    (int)host.len, host.ptr);
    } 
    else if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message*)ev_data;
        
        if (ctx->callback) {
            ctx->callback(hm->body.ptr, hm->body.len, ctx->user_data);
        }
        
        ctx->completed = true;
        c->is_closing = 1;
    } 
    else if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE) {
        ctx->completed = true;
    }
}

void mg_fetch_async(struct mg_mgr *mgr, const char *url, 
                   response_callback callback, void *user_data) {
    request_context *ctx = (request_context *)malloc(sizeof(request_context));
    if (ctx == NULL) return;
    
    ctx->callback = callback;
    ctx->user_data = user_data;
    ctx->completed = false;
    ctx->url_copy = strdup(url);

    if (ctx->url_copy == NULL) {
        free(ctx);
        return;
    }

    struct mg_connection *c = mg_http_connect(mgr, url, http_callback, ctx);
    if (c == NULL) {
        free(ctx->url_copy);
        free(ctx);
    }
}

void mg_cleanup_requests(struct mg_mgr *mgr) {
    mg_mgr_free(mgr);
}