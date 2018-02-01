@echo off
setlocal

rem For Visual Studio 2015
set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC

rem For x86_64
set PATH=%VCINSTALLDIR%\bin\amd64;%PATH%
set INCLUDE=%VCINSTALLDIR%\include;%INCLUDE%
set LIBPATH=%VCINSTALLDIR%\lib\amd64;%LIBPATH%
set LIB=%VCINSTALLDIR%\lib\amd64;%LIB%

set opencl_include=C:\Program Files\CUDA\9.1\include
set opencl_libpath=C:\Program Files\CUDA\9.1\lib\Win32

set cflags=/nologo /Zi /MT /I"%opencl_include%"
set defines=/DPLATFORM_WIN32=1 /DPROGRAM_NAME=\"blur\" /DENABLE_ASSERT=0

set lflags=/debug /incremental:no /libpath:""%opencl_libpath%"
set libs=OpenCL.lib


if not exist "bin/" mkdir "bin/"

echo "Building 'blur'"
cl /Fe"bin\blur.exe" "src\blur.c" %cflags% %defines% /link %lflags% %libs%
