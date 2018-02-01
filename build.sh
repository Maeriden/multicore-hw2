#! /bin/sh

[ -d "bin/" ] || mkdir "bin/"

cflags="-O2"
defines="-DPLATFORM_LINUX=1 -DPROGRAM_NAME=\"blur\" -DENABLE_ASSERT=0"

lflags=""
libs="-lOpenCL"

# echo "Building 'blur'"
gcc -o bin/blur ${cflags} ${defines} ${lflags} ${libs} src/blur.c
