#! /bin/sh

[ -d "bin/" ] || mkdir "bin/"
[ -d "obj/" ] || mkdir "obj/"

cflags="-g3 -O0"
defines="-DPLATFORM_LINUX=1 -DPROGRAM_NAME=\"blur\" -DENABLE_ASSERT=1"

lflags=""
libs="-lOpenCL"

echo "Building 'blur'"
gcc -o bin/blur ${cflags} ${defines} ${lflags} ${libs} src/blur.c
