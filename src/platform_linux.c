#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

static
size_t
linux_get_file_size(const char* path)
{
	struct stat st = {};
	int error = stat(path, &st);
	if(error != 0)
		perror("linux_get_file_size");
	return st.st_size;
}


static
char*
linux_get_executable_path(const char* argv0)
{
	pid_t pid      = getpid();
	char  proc[32] = {0};
	sprintf(proc, "/proc/%d/exe", pid);
	
	char    buffer[1024] = {0};
	ssize_t read_count   = readlink(proc, buffer, sizeof(buffer));
	ASSERT(read_count != -1);
	
	char* result = malloc(read_count+1);
	strncpy(result, buffer, read_count+1);
	return result;
}


static
char*
linux_get_dirname(const char* path)
{
	size_t path_len = strlen(path);
	char*  result   = malloc(path_len + 1);
	strncpy(result, path, path_len+1);
	return dirname(result);
}
