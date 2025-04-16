CC = gcc
TARGET = api
PROG ?= $(TARGET)              
DELETE = rm -rf                
OUT ?= -o $(PROG)              
SOURCES = main.c mongoose/mongoose.c 
OBJS = $(SOURCES:.c=.o)

# Common flags for both dev and prod
COMMON_CFLAGS = -Wall -Wextra -I. -std=gnu23
COMMON_CFLAGS += -Wformat=2 \
-Wformat-truncation \
-Wformat-overflow \
-Wconversion \
-Wduplicated-cond \
-Wduplicated-branches \
-Wlogical-op \
-Wnull-dereference \
-Wjump-misses-init \
-Wshadow \
-Wpointer-arith \
-Wcast-align \
-Wcast-qual \
-Wwrite-strings \
-Wmissing-prototypes \
-Wmissing-declarations \
-Wredundant-decls

# Development configuration
DEV_CFLAGS = $(COMMON_CFLAGS)
DEV_CFLAGS += -O0 -g -fanalyzer

# Production configuration
PROD_CFLAGS = $(COMMON_CFLAGS)
PROD_CFLAGS += -O3 -flto=auto -march=native -ffast-math -funroll-loops -DNDEBUG -fomit-frame-pointer

# Mongoose-specific flags
CFLAGS_MONGOOSE = -DMG_HTTP_DIRLIST_TIME_FMT="%Y/%m/%d %H:%M:%S"
CFLAGS_MONGOOSE += -DMG_ENABLE_LINES=0 -DMG_ENABLE_IPV6=1 -DMG_ENABLE_SSI=1
CFLAGS_MONGOOSE += -DMG_IO_SIZE=8192
CFLAGS_MONGOOSE += -DMG_ENABLE_DIRECTORY_LISTING=0
CFLAGS_MONGOOSE += -DMG_ENABLE_FILESYSTEM=0  # If you don't serve files

CFLAGS_EXTRA ?= -DMG_TLS=MG_TLS_BUILTIN

# Development linker flags
DEV_LDFLAGS = -pthread -lsqlite3 -lm

# Production linker flags
PROD_LDFLAGS = -pthread -lsqlite3 -lm -flto=auto -Wl,-O3,--as-needed,--gc-sections

# Default to dev build
CFLAGS = $(DEV_CFLAGS)
LDFLAGS = $(DEV_LDFLAGS)

all: dev

dev: CFLAGS = $(DEV_CFLAGS)
dev: LDFLAGS = $(DEV_LDFLAGS)
dev: $(PROG)

prod: CFLAGS = $(PROD_CFLAGS)
prod: LDFLAGS = $(PROD_LDFLAGS)
prod: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) -c $< -o $@

pgo-gen: clean
	$(MAKE) CFLAGS="$(PROD_CFLAGS) -fprofile-generate" LDFLAGS="$(PROD_LDFLAGS) -fprofile-generate"

pgo-use: clean
	$(MAKE) CFLAGS="$(PROD_CFLAGS) -fprofile-use -fprofile-correction" LDFLAGS="$(PROD_LDFLAGS) -fprofile-use"

clean:
	$(DELETE) $(PROG) $(OBJS) *.gcda *.gcno

run: $(PROG)
	./$(PROG)

.PHONY: all clean run dev prod pgo-gen pgo-use