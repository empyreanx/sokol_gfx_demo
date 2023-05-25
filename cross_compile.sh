#!/bin/bash

shopt -s nullglob
for f in *.glsl; do
	echo "Compiling $f to ${f%%.*}_shader.h"
	./sokol-shdc --input "$f" --output "${f%%.*}_shader.h" --slang glsl330:hlsl5:metal_macos:metal_ios:metal_sim:glsl300es:glsl100 --reflection
done
