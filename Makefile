CC = gcc
TARGET = api
PROG ?= $(TARGET)              
DELETE = rm -rf                
OUT ?= -o $(PROG)              
SOURCES = main.c mongoose/mongoose.c 
OBJS = $(SOURCES:.c=.o)

CFLAGS = -Wall -Wextra -I. -std=gnu17
CFLAGS += -O3 -flto=auto -march=native -ffast-math -funroll-loops
CFLAGS += -g -fanalyzer


CFLAGS += -Wformat=2 \
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

CFLAGS_MONGOOSE = -DMG_HTTP_DIRLIST_TIME_FMT="%Y/%m/%d %H:%M:%S"
CFLAGS_MONGOOSE += -DMG_ENABLE_LINES=0 -DMG_ENABLE_IPV6=1 -DMG_ENABLE_SSI=1
CFLAGS_MONGOOSE += -DMG_IO_SIZE=8192
CFLAGS_MONGOOSE += -DMG_ENABLE_DIRECTORY_LISTING=0
CFLAGS_MONGOOSE += -DMG_ENABLE_FILESYSTEM=0  # If you don't serve files

CFLAGS_EXTRA ?= -DMG_TLS=MG_TLS_BUILTIN
LDFLAGS = -pthread -lsqlite3 -lm -flto=auto -Wl,-O3,--as-needed,--gc-sections

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) -c $< -o $@


production: CFLAGS := $(filter-out -g -fanalyzer,$(CFLAGS))
production: CFLAGS += -DNDEBUG -fomit-frame-pointer
production: $(PROG)

pgo-gen: clean
	$(MAKE) CFLAGS="$(CFLAGS) -fprofile-generate" LDFLAGS="$(LDFLAGS) -fprofile-generate"

pgo-use: clean
	$(MAKE) CFLAGS="$(CFLAGS) -fprofile-use -fprofile-correction" LDFLAGS="$(LDFLAGS) -fprofile-use"

clean:
	$(DELETE) $(PROG) $(OBJS) *.gcda *.gcno

run: $(PROG)
	./$(PROG)

.PHONY: all clean run production pgo-gen pgo-use