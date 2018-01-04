#include <windows.h>


static
size_t
win32_get_file_size(const char* path)
{
	WIN32_FILE_ATTRIBUTE_DATA attrs = {};
	BOOL success = GetFileAttributesEx(path, GetFileExInfoStandard, &attrs);
	size_t size = (attrs.nFileSizeHigh << 32) | (attrs.nFileSizeLow)
	return size;
}


static
char*
win32_get_executable_path(const char* argv0)
{
	char buffer[1024];
	DWORD path_len = GetModuleFileName(NULL, buffer, sizeof(buffer));
	
	char* result = malloc(path_len+1);
	strncpy(result, buffer, path_len);
	return result;
}


static
char*
win32_get_dirname(const char* path)
{
	char buffer[_MAX_DIR];
	errno_t error = _splitpath_s(path,
	                             NULL, 0,
	                             buffer, sizeof(buffer),
	                             NULL, 0,
	                             NULL, 0);
	char* result = NULL;
	if(error != EINVAL)
	{
		size_t result_len = strlen(buffer);
		result = malloc(result_len+1);
		strncpy(result, buffer, result_len+1);
	}
	return result;
}
