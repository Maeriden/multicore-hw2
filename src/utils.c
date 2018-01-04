#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE 1
#endif


#if !defined(DEBUGBREAK)
	#if defined(__clang__)
		#define DEBUGBREAK __builtin_trap()
	#elif defined(__GNUC__)
		#define DEBUGBREAK __asm__ __volatile__ ("int3")
	#elif defined(_MSC_VER)
		#define DEBUGBREAK __debugbreak()
	#endif
#endif

#if ENABLE_ASSERT
	#include <stdio.h>
	#define ASSERT(c) do{ if(!(c)) {fprintf(stderr, "%s:%d\n", __FILE__, __LINE__); DEBUGBREAK;} } while(0)
#else
	#define ASSERT(c) do{ (void)sizeof(c); } while(0)
#endif

#if ENABLE_ASSERT
	#include <stdio.h>
	#define CL_ASSERT(c) do{ if(!(c)) {fprintf(stderr, "[%s:%d] %s\n", __FILE__, __LINE__, clGetErrorString(error)); DEBUGBREAK;} } while(0)
#else
	#define CL_ASSERT(c) do{ (void)sizeof(c); } while(0)
#endif


#define ABS(n) ((n) < 0 ? -(n) : (n))


static
char*
strconcat(const char* s1, const char* s2)
{
	size_t s1_len = strlen(s1);
	size_t s2_len = strlen(s2);
	
	char* result = malloc(s1_len + s2_len + 1);
	strncpy(result,          s1, s1_len);
	strncpy(result + s1_len, s2, s2_len);
	result[s1_len + s2_len] = 0;
	return result;
}


static
void
debug_blur(unsigned char* source,
           int source_w,
           int source_h,
           unsigned char* target,
           int target_w,
           int target_h,
           unsigned char* mask,
           int mask_radius)
{
	for(int y = 0; y < source_h; ++y)
	{
		for(int x = 0; x < source_w; ++x)
		{
			if(0 + mask_radius <= x && x < source_w-mask_radius
			&& 0 + mask_radius <= y && y < source_h-mask_radius)
			{
				int mask_side = 2*mask_radius + 1;
				int c         = 0;
				int n         = 0;
				
				for(int row = 0; row < mask_side; ++row)
				{
					for(int col = 0; col < mask_side; ++col)
					{
						int px = x + col - mask_radius;
						int py = y + row - mask_radius;
						
						if(0 <= px && px < source_w
						&& 0 <= py && py < source_h)
						{
							if(mask[row*mask_side + col] > 0)
							{
								c += source[py*source_w + px] * mask[row*mask_side + col];
								n += 1;
							}
						}
					}
				}
				
				if(n > 0)
				{
					unsigned char color = (unsigned char)((float)c / (float)n);
					target[y*source_w + x] = color;
				}
			}
		}
	}
}


static
const char*
clGetErrorString(int error)
{
	switch(error)
	{
		case     0: return "CL_SUCCESS";
		case    -1: return "CL_DEVICE_NOT_FOUND";
		case    -2: return "CL_DEVICE_NOT_AVAILABLE";
		case    -3: return "CL_COMPILER_NOT_AVAILABLE";
		case    -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case    -5: return "CL_OUT_OF_RESOURCES";
		case    -6: return "CL_OUT_OF_HOST_MEMORY";
		case    -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case    -8: return "CL_MEM_COPY_OVERLAP";
		case    -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case   -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case   -12: return "CL_MAP_FAILURE";
		case   -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case   -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case   -15: return "CL_COMPILE_PROGRAM_FAILURE";
		case   -16: return "CL_LINKER_NOT_AVAILABLE";
		case   -17: return "CL_LINK_PROGRAM_FAILURE";
		case   -18: return "CL_DEVICE_PARTITION_FAILED";
		case   -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
		case   -30: return "CL_INVALID_VALUE";
		case   -31: return "CL_INVALID_DEVICE_TYPE";
		case   -32: return "CL_INVALID_PLATFORM";
		case   -33: return "CL_INVALID_DEVICE";
		case   -34: return "CL_INVALID_CONTEXT";
		case   -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case   -36: return "CL_INVALID_COMMAND_QUEUE";
		case   -37: return "CL_INVALID_HOST_PTR";
		case   -38: return "CL_INVALID_MEM_OBJECT";
		case   -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case   -40: return "CL_INVALID_IMAGE_SIZE";
		case   -41: return "CL_INVALID_SAMPLER";
		case   -42: return "CL_INVALID_BINARY";
		case   -43: return "CL_INVALID_BUILD_OPTIONS";
		case   -44: return "CL_INVALID_PROGRAM";
		case   -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case   -46: return "CL_INVALID_KERNEL_NAME";
		case   -47: return "CL_INVALID_KERNEL_DEFINITION";
		case   -48: return "CL_INVALID_KERNEL";
		case   -49: return "CL_INVALID_ARG_INDEX";
		case   -50: return "CL_INVALID_ARG_VALUE";
		case   -51: return "CL_INVALID_ARG_SIZE";
		case   -52: return "CL_INVALID_KERNEL_ARGS";
		case   -53: return "CL_INVALID_WORK_DIMENSION";
		case   -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case   -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case   -56: return "CL_INVALID_GLOBAL_OFFSET";
		case   -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case   -58: return "CL_INVALID_EVENT";
		case   -59: return "CL_INVALID_OPERATION";
		case   -60: return "CL_INVALID_GL_OBJECT";
		case   -61: return "CL_INVALID_BUFFER_SIZE";
		case   -62: return "CL_INVALID_MIP_LEVEL";
		case   -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case   -64: return "CL_INVALID_PROPERTY";
		case   -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case   -66: return "CL_INVALID_COMPILER_OPTIONS";
		case   -67: return "CL_INVALID_LINKER_OPTIONS";
		case   -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";
		case   -69: return "CL_INVALID_PIPE_SIZE";
		case   -70: return "CL_INVALID_DEVICE_QUEUE";
		case   -71: return "CL_INVALID_SPEC_ID";
		case   -72: return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
		case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
		case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
		case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
		case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
		case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
		case -1006: return "CL_INVALID_D3D11_DEVICE_KHR";
		case -1007: return "CL_INVALID_D3D11_RESOURCE_KHR";
		case -1008: return "CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR";
		case -1009: return "CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR";
		case -1010: return "CL_INVALID_DX9_MEDIA_ADAPTER_KHR";
		case -1011: return "CL_INVALID_DX9_MEDIA_SURFACE_KHR";
		case -1012: return "CL_DX9_MEDIA_SURFACE_ALREADY_ACQUIRED_KHR";
		case -1013: return "CL_DX9_MEDIA_SURFACE_NOT_ACQUIRED_KHR";
		case -1093: return "CL_INVALID_EGL_OBJECT_KHR";
		case -1092: return "CL_EGL_RESOURCE_NOT_ACQUIRED_KHR";
		case -1057: return "CL_DEVICE_PARTITION_FAILED_EXT";
		case -1058: return "CL_INVALID_PARTITION_COUNT_EXT";
		case -1059: return "CL_INVALID_PARTITION_NAME_EXT";
		case -1094: return "CL_INVALID_ACCELERATOR_INTEL";
		case -1095: return "CL_INVALID_ACCELERATOR_TYPE_INTEL";
		case -1096: return "CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL";
		case -1097: return "CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL";
		case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case -1098: return "CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL";
		case -1099: return "CL_INVALID_VA_API_MEDIA_SURFACE_INTEL";
		case -1100: return "CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL";
		case -1101: return "CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL";
		default:    return "CL_UNKNOWN_ERROR";
	}
}
