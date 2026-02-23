CC=gcc
AR=ar

EXE=creaturettes.exe

CFLAGS=-o $(EXE) -Wall -Wextra -std=c99 -Iraylib/src

SOURCES=src/main.c raylib.a

ifeq ($(OS), Windows_NT)
	CMD=$(CC) $(SOURCES) $(CFLAGS) -lgd32 -lwinmm
else
	CMD=$(CC) $(SOURCES) $(CFLAGS) -lm
endif

# If we're using TCC, we disable SIMD
ifeq ($(CC), tcc)
	CONFIG_CMD=$(shell echo "#define STBIR_NO_SIMD 1" >> raylib/src/config.h )
else
	CONFIG_CMD=
endif

build: raylib.a
	$(CMD) -O0 -g -DPLATFORM_DESKTOP

release: raylib.a
	$(CMD) -O3 -s -DRELEASE -DPLATFORM_DESKTOP

clang:
	make release CC=clang

gcc:
	make release CC=gcc

tcc:
	make release CC=tcc AR="tcc -ar"

run: build
	./$(EXE)

raylib:
	git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib

raylib.a: raylib
	$(CONFIG_CMD)
	make -C raylib/src CC="$(CC)" AR="$(AR)" RAYLIB_MODULE_MODELS=FALSE
	cp raylib/src/libraylib.a raylib.a

clean:
	make -C raylib/src clean
	rm -f *.a *.o $(EXE)
