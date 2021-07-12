#!/usr/bin/env bash

name=website
source=source/main.c
compiler=gcc

echo "Building..." && \
$compiler \
    -std=c99 \
    -g -Os -static -nostdlib -nostdinc -fno-pie \
    -mno-red-zone -fno-omit-frame-pointer -pg \
    -o $name.com.dbg $source  -fuse-ld=bfd -Wl,-T,buildkit/cosmo/ape.lds \
    -include buildkit/cosmo/cosmopolitan.h buildkit/cosmo/crt.o \
    buildkit/cosmo/ape.o buildkit/cosmo/cosmopolitan.a && \
objcopy -S -O binary $name.com.dbg $name.com && \
echo "Built!"
