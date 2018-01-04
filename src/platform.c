#ifdef __APPLE__
	#include <OpenCL/openCL.h>
#else
	#include <CL/cl.h>
#endif


// Redefinition to 0 will throw an error if multiple platforms are defined
#if PLATFORM_LINUX
	#define PLATFORM_MACOS 0
	#define PLATFORM_WIN32 0
	#include "platform_linux.c"
#endif

#if PLATFORM_MACOS
	#define PLATFORM_LINUX 0
	#define PLATFORM_WIN32 0
	#include "platform_macos.c"
#endif

#if PLATFORM_WIN32
	#define PLATFORM_LINUX 0
	#define PLATFORM_MACOS 0
	#include "platform_win32.c"
#endif


static
size_t
get_file_size(const char* path)
{
#if PLATFORM_LINUX
	return linux_get_file_size(path);
#endif
#if PLATFORM_MACOS
	return macos_get_file_size(path);
#endif
#if PLATFORM_WIN32
	return win32_get_file_size(path);
#endif
}


static
char*
get_executable_path(const char* argv0)
{
#if PLATFORM_LINUX
	return linux_get_executable_path(argv0);
#endif
#if PLATFORM_MACOS
	return macos_get_executable_path(argv0);
#endif
#if PLATFORM_WIN32
	return win32_get_executable_path(argv0);
#endif
}


static
char*
get_dirname(const char* path)
{
#if PLATFORM_LINUX
	return linux_get_dirname(path);
#endif
#if PLATFORM_MACOS
	return macos_get_dirname(path);
#endif
#if PLATFORM_WIN32
	return win32_get_dirname(path);
#endif
}
