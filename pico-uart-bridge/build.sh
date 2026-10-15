#!/bin/sh
# Build pico_uart_bridge.uf2 inside Docker; the result is copied to ./bin.
set -e

cd "$(dirname "$0")"

docker build -t pico-uart-bridge-builder .
mkdir -p bin

docker run --rm -v "$PWD":/src:ro -v "$PWD/bin":/out pico-uart-bridge-builder sh -ec '
	cmake -S /src -B /tmp/build
	make -C /tmp/build -j"$(nproc)"
	cp /tmp/build/pico_uart_bridge.uf2 /out/
'
