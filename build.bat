@echo off

set name=website
set source=source\main.c

echo *** Building source...
buildkit\cross9\bin\x86_64-pc-linux-gnu-gcc.exe ^
    -std=c99 ^
    -g -O -o %name%.com.dbg ^
    %source% ^
    -static -fno-pie -no-pie -mno-red-zone -fno-omit-frame-pointer -nostdlib -nostdinc ^
    -Wl,--gc-sections -Wl,-z,max-page-size=0x1000 -fuse-ld=bfd ^
    -Wl,-T,buildkit\cosmo\ape.lds -include buildkit\cosmo\cosmopolitan.h buildkit\cosmo\crt.o buildkit\cosmo\ape.o buildkit\cosmo\cosmopolitan.a && ^
buildkit\cross9\bin\x86_64-pc-linux-gnu-objcopy.exe -SO binary %name%.com.dbg %name%.com && ^
echo *** Source built!