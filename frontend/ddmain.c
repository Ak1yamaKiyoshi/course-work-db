
#include "src/utils/fetch.c"

void on_response(const char* data, size_t length, void* user_data) {
    char* buffer = (char*)user_data;
    if (buffer && data && length > 0) {
        memcpy(buffer, data, length < 4096 ? length : 4096);
        buffer[length < 4096 ? length : 4095] = '\0';
    }
    printf("Response received, %zu bytes\n", length);
}

int main(void) {
    struct mg_mgr mgr;
    char response_buffer[4096] = {0};
    
    mg_mgr_init(&mgr);
    
    mg_fetch_async(&mgr, "http://127.0.0.1:8000/api/categories", 
                  on_response, response_buffer);
    
    for (int i = 0; i < 100 && response_buffer[0] == 0; i++) {
        mg_mgr_poll(&mgr, 100); 
    }
    
    if (response_buffer[0] != 0) {
        printf("Final response: %s\n", response_buffer);
    } else {
        printf("Request failed or timed out\n");
    }
    
    mg_mgr_free(&mgr);
    return 0;
}