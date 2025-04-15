#pragma once


struct product {
    size_t id; 
    char *name;
    char *description;
    double price; 
    char *type;
    char *image;
    char *size;
    char *is_custom;
};    


struct result {
    size_t ptr;
    size_t capacity; 
    struct product *array;
};