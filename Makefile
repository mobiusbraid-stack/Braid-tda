# Makefile for @braidmesh/tda WebAssembly core
CC = emcc
CFLAGS = -O3 --bind -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s EXPORT_NAME="BraidMeshTdaWasm"

OUT_DIR = ./dist/wasm

all: $(OUT_DIR)/tda_core.js

$(OUT_DIR)/tda_core.js: cpp/tda_core.cpp
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) cpp/tda_core.cpp -o $(OUT_DIR)/tda_core.js

clean:
	rm -rf $(OUT_DIR)
