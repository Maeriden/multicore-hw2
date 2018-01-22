@echo off
setlocal

rem Check that the compiler is in the PATH
cl /? >NUL 2>NUL

if not %ERRORLEVEL% == 0 (
	call vcvarsall.bat x64 2>NUL

	if not %ERRORLEVEL% == 0 (
		echo Cannot find MSVC compiler (cl.exe)
		echo Run again from a Visual Studio command prompt, or add vcvarsall.bat to the PATH
		goto exit
	)
)

if not exist "bin/" mkdir "bin/"
if not exist "obj/" mkdir "obj/"

set cflags=/nologo /Zi /MTd /I"C:\Program Files\CUDA\9.1\include"
set defines=/DPLATFORM_WIN32=1 /DPROGRAM_NAME=\"blur\" /DENABLE_ASSERT=1

set lflags=/debug /incremental:no /libpath:"C:\Program Files\CUDA\9.1\lib\Win32"
set libs=OpenCL.lib

echo "Building 'blur'"
cl /Fe"bin\blur.exe" src\blur.c %cflags% %defines% /link %lflags% %libs%

:exit
