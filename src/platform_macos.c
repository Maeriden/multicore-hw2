#include <unistd.h>
#include <sys/stat.h>

static
size_t
macos_get_file_size(const char* path)
{
	struct stat st = {};
	int error = stat(path, &st);
	return st.st_size;
}


static
char*
macos_get_executable_path(const char* argv0)
{
	uint32_t bufsize = 0;
	int error = _NSGetExecutablePath(NULL, &bufsize);
	if(error == -1)
	{
		char* buffer = malloc(bufsize);
		error = _NSGetExecutablePath(buffer, &bufsize);
		if(error == 0)
		{
			return buffer;
		}
		free(buffer);
	}
	
	return NULL;
}


static
char*
macos_get_dirname(const char* path)
{
	size_t path_len = strlen(path);
	char*  result   = malloc(path_len + 1);
	strncpy(result, path, path_len+1);
	return dirname(result);
}
