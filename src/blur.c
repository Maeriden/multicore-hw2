#include "argtable3.c"
#include "pgm.c"
#include "utils.c"
#include "platform.c"


#define ENABLE_USE_IMAGE2D 0

#ifndef PROGRAM_NAME
	#define PROGRAM_NAME "blur"
#endif


int
main(int argc, char** argv)
{
	int         blur_radius = 1;
	const char* source_path = NULL;
	const char* output_path = NULL;
	
	// Parse command line arguments
	{
		struct arg_lit*  argt_help        = arg_litn("h?", "help", 0, 1, "Print this help and exit");
		struct arg_int*  argt_blur_radius = arg_intn("l", "level", "{1,2,3,4}", 0, 1, "Blur level (default: 1)");
		struct arg_file* argt_source_file = arg_filen(NULL, NULL, "SOURCE", 1, 1, "Source image path");
		struct arg_file* argt_output_file = arg_filen(NULL, NULL, "OUTPUT", 1, 1, "Output image path");
		struct arg_end*  argt_end         = arg_end(20);
		void*            argt_argtable[]  = {argt_help, argt_blur_radius, argt_source_file, argt_output_file, argt_end};
		
		if(arg_nullcheck(argt_argtable) != 0)
		{
			// TODO: Error reporting
			return 1;
		}
		
		int argt_error_count = arg_parse(argc, argv, argt_argtable);
		
		// Help option takes precedence over error checking
		if(argt_help->count)
		{
			fputs("Usage: "PROGRAM_NAME" [-l N] SOURCE OUTPUT\n", stderr);
			// arg_print_syntax(stderr, argt_argtable, "\n");
			arg_print_glossary_gnu(stderr, argt_argtable);
			arg_freetable(argt_argtable, sizeof(argt_argtable) / sizeof(argt_argtable[0]));
			return 0;
		}
		
		if(argt_error_count > 0)
		{
			arg_print_errors(stderr, argt_end, PROGRAM_NAME);
			fputs("Try '"PROGRAM_NAME" --help' for more information\n", stderr);
			return 1;
		}
		
		if(argt_blur_radius->count > 0) blur_radius = argt_blur_radius->ival[0];
		if(argt_source_file->count > 0) source_path = argt_source_file->filename[0];
		if(argt_output_file->count > 0) output_path = argt_output_file->filename[0];
	
		arg_freetable(argt_argtable, sizeof(argt_argtable) / sizeof(argt_argtable[0]));
	}
	
	
	if(blur_radius < 1)
	{
		fputs(PROGRAM_NAME": blur level must be greater than 0\n", stderr);
		return 1;
	}
	
	
	unsigned char* source_image         = NULL;
	int            source_image_size[2] = {0, 0};
	unsigned char* output_image         = NULL;
	int            output_image_size[2] = {0, 0};
	
	// Load image into memory
	{
		unsigned char* temp;
		int rows, cols;
		int error = pgm_load(&temp, &rows, &cols, source_path);
		if(error)
		{
			fputs("Could not load source image\n", stderr);
			return 1;
		}
		
		source_image = temp;
		source_image_size[0] = cols;
		source_image_size[1] = rows;
		
		output_image = calloc(cols*rows, sizeof(*output_image));
		output_image_size[0] = cols;
		output_image_size[1] = rows;
	}
	
	
	// Set blur mask up
	float* blur_mask = NULL;
	{
		size_t mask_side = 2*blur_radius + 1;
		blur_mask = calloc(mask_side*mask_side, sizeof(*blur_mask));
		
		for(int row = 0; row < mask_side; ++row)
		{
			for(int col = 0; col < mask_side; ++col)
			{
				// Distance of mask[row, col] from the center of the mask
				int distance = ABS(col-blur_radius) + ABS(row-blur_radius);
				
				if(distance <= blur_radius)
					blur_mask[row*mask_side + col] = 1;
				else
					blur_mask[row*mask_side + col] = 0;
			}
		}
	}
	
	
	// Set opencl environment up
	cl_platform_id   platform_id;
	cl_device_id     device_id;
	cl_context       context;
	cl_command_queue command_queue;
	{
		cl_int error = 0;
		
		error = clGetPlatformIDs(1, &platform_id, NULL);
		CL_ASSERT(error == CL_SUCCESS);
		
		error = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
		CL_ASSERT(error == CL_SUCCESS);
		
		context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &error);
		CL_ASSERT(error == CL_SUCCESS);
		
		command_queue = clCreateCommandQueueWithProperties(context, device_id, 0, &error);
		CL_ASSERT(error == CL_SUCCESS);
	}
	
	
	// Get kernels source file
	char* source_file_path = NULL;
	{
		char* executable_path = get_executable_path(argv[0]);
		char* executable_dir  = get_dirname(executable_path);
		
		source_file_path = strconcat(executable_dir, "/kernels/blur.cl");
		free(executable_path);
		free(executable_dir);
	}
	
	
	// Get kernels source
	char* program_source = NULL;
	{
		size_t source_size = get_file_size(source_file_path);
		if(source_size == 0)
		{
			fputs("Error reading kernel source code\n", stderr);
			return 1;
		}
		
		program_source    = malloc(source_size);
		FILE* source_file = fopen(source_file_path, "rb");
		free(source_file_path);
		
		size_t read_count = fread(program_source, sizeof(char), source_size, source_file);
		fclose(source_file);
		if(read_count != source_size)
		{
			fputs("Error reading kernel source code\n", stderr);
			return 1;
		}
	}
	
	
	// Build program
	cl_program program;
	{
		cl_int error = 0;
		
		program = clCreateProgramWithSource(context, 1, (const char**)&program_source, NULL, &error);
		CL_ASSERT(error == CL_SUCCESS);
		free(program_source);
		
		error = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
		if(error != CL_SUCCESS)
		{
			char   log[8192];
			size_t len;
			clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(log), log, &len);
			fprintf(stderr, "%s\n", log);
			return 1;
		}
	}
	
	
	// Set memory up
#if ENABLE_USE_IMAGE2D
	cl_mem cl_source_image;
	{
		cl_int error = 0;
		
		cl_mem_flags flags = CL_MEM_READ_ONLY
		                   | CL_MEM_COPY_HOST_PTR
		                   | CL_MEM_HOST_WRITE_ONLY;
		
		cl_image_format format = {};
		format.image_channel_order     = CL_INTENSITY;
		format.image_channel_data_type = CL_UNSIGNED_INT8;
		
		cl_image_desc desc = {};
		desc.image_type        = CL_MEM_OBJECT_IMAGE2D;
		desc.image_width       = source_image_size[0];
		desc.image_height      = source_image_size[1];
		desc.image_depth       = 0; // only for 3D images
		desc.image_array_size  = 0; // only used if the image is a 1D or 2D image array
		desc.image_row_pitch   = source_image_size[0] * sizeof(*source_image);
		desc.image_slice_pitch = 0; // only for 3D images
		desc.num_mip_levels    = 0; // Must be 0
		desc.num_samples       = 0; // Must be 0
		desc.buffer            = NULL;
		
		ASSERT(source_image);
		cl_source_image = clCreateImage(context, flags, &format, &desc, source_image, &error);
		CL_ASSERT(error == CL_SUCCESS);
	}
	
	cl_mem cl_output_image;
	{
		cl_int error = 0;
		
		cl_mem_flags flags = CL_MEM_WRITE_ONLY
		                   | CL_MEM_USE_HOST_PTR
		                   | CL_MEM_HOST_READ_ONLY;
		
		cl_image_format format = {};
		format.image_channel_order     = CL_INTENSITY;
		format.image_channel_data_type = CL_UNSIGNED_INT8;
		
		cl_image_desc desc = {};
		desc.image_type        = CL_MEM_OBJECT_IMAGE2D;
		desc.image_width       = output_image_size[0];
		desc.image_height      = output_image_size[1];
		desc.image_depth       = 0; // only for 3D images
		desc.image_array_size  = 0; // only used if the image is a 1D or 2D image array
		desc.image_row_pitch   = output_image_size[0] * sizeof(*output_image);
		desc.image_slice_pitch = 0; // only for 3D images
		desc.num_mip_levels    = 0; // Must be 0
		desc.num_samples       = 0; // Must be 0
		desc.buffer            = NULL;
		
		ASSERT(output_image);
		cl_output_image = clCreateImage(context, flags, &format, &desc, output_image, &error);
		CL_ASSERT(error == CL_SUCCESS);
	}
#else
	cl_mem cl_source_image;
	cl_int cl_source_image_w;
	cl_int cl_source_image_h;
	{
		cl_int error = 0;
		
		cl_mem_flags flags = CL_MEM_READ_ONLY
		                   | CL_MEM_COPY_HOST_PTR
		                   | CL_MEM_HOST_WRITE_ONLY;
		
		size_t buffer_size = source_image_size[0]*source_image_size[1] * sizeof(*source_image);
		cl_source_image  = clCreateBuffer(context, flags, buffer_size, source_image, &error);
		cl_source_image_w = source_image_size[0];
		cl_source_image_h = source_image_size[1];
		CL_ASSERT(error == CL_SUCCESS);
	}
	
	cl_mem cl_output_image;
	cl_int cl_output_image_w;
	cl_int cl_output_image_h;
	{
		cl_int error = 0;
		
		cl_mem_flags flags = CL_MEM_WRITE_ONLY
		                   | CL_MEM_USE_HOST_PTR
		                   | CL_MEM_HOST_READ_ONLY;
		
		size_t buffer_size = output_image_size[0]*output_image_size[1] * sizeof(*output_image);
		cl_output_image  = clCreateBuffer(context, flags, buffer_size, output_image, &error);
		cl_output_image_w = output_image_size[0];
		cl_output_image_h = output_image_size[1];
		CL_ASSERT(error == CL_SUCCESS);
	}
#endif
	
	cl_mem cl_blur_mask;
	cl_int cl_blur_mask_radius;
	{
		cl_int error = 0;
		
		cl_blur_mask_radius = blur_radius;
		
		cl_mem_flags flags = CL_MEM_READ_ONLY
		                   | CL_MEM_COPY_HOST_PTR
		                   | CL_MEM_HOST_WRITE_ONLY;
		
		size_t mask_side = 2*blur_radius + 1;
		cl_blur_mask  = clCreateBuffer(context, flags, mask_side*mask_side * sizeof(*blur_mask), blur_mask, &error);
		CL_ASSERT(error == CL_SUCCESS);
	}
	
	
	// Set kernel up
	cl_kernel kernel;
	{
		cl_int error = 0;
		
		int arg_index = 0;
#if ENABLE_USE_IMAGE2D
		const char* kernel_name = "blur_image";
		
		kernel = clCreateKernel(program, kernel_name, &error);
		CL_ASSERT(error == CL_SUCCESS);
		
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_source_image), &cl_source_image);
		CL_ASSERT(error == CL_SUCCESS);
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_output_image), &cl_output_image);
		CL_ASSERT(error == CL_SUCCESS);
		
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_blur_mask), &cl_blur_mask);
		CL_ASSERT(error == CL_SUCCESS);
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_blur_mask_radius), &cl_blur_mask_radius);
		CL_ASSERT(error == CL_SUCCESS);
		
		size_t origin[] = {0, 0, 0};
		size_t region[] = {source_image_size[0], source_image_size[1], 1};
		error = clEnqueueWriteImage(command_queue, cl_source_image, CL_FALSE,
		                            origin, region, 0, 0, source_image,
		                            0, NULL, NULL);
		CL_ASSERT(error == CL_SUCCESS);
#else
		const char* kernel_name = "blur";
		
		kernel = clCreateKernel(program, kernel_name, &error);
		CL_ASSERT(error == CL_SUCCESS);
		
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_source_image), &cl_source_image);
		CL_ASSERT(error == CL_SUCCESS);
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_source_image_w), &cl_source_image_w);
		CL_ASSERT(error == CL_SUCCESS);
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_source_image_h), &cl_source_image_h);
		CL_ASSERT(error == CL_SUCCESS);
		
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_output_image), &cl_output_image);
		CL_ASSERT(error == CL_SUCCESS);
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_output_image_w), &cl_output_image_w);
		CL_ASSERT(error == CL_SUCCESS);
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_output_image_h), &cl_output_image_h);
		CL_ASSERT(error == CL_SUCCESS);
		
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_blur_mask), &cl_blur_mask);
		CL_ASSERT(error == CL_SUCCESS);
		error = clSetKernelArg(kernel, arg_index++, sizeof(cl_blur_mask_radius), &cl_blur_mask_radius);
		CL_ASSERT(error == CL_SUCCESS);
		
		size_t buffer_size = source_image_size[0]*source_image_size[1] * sizeof(*source_image);
		error = clEnqueueWriteBuffer(command_queue, cl_source_image, CL_FALSE,
		                             0, buffer_size, source_image,
		                             0, NULL, NULL);
		CL_ASSERT(error == CL_SUCCESS);
#endif
	}
	
	
	// Execute & read result
	{
		cl_int error = 0;
		
		size_t global_work_size[] = {source_image_size[0], source_image_size[1]};
		error = clEnqueueNDRangeKernel(command_queue, kernel, 2,
		                               NULL, global_work_size, NULL,
		                               0, NULL, NULL);
		CL_ASSERT(error == CL_SUCCESS);
		
#if ENABLE_USE_IMAGE2D
		size_t origin[] = {0, 0, 0};
		size_t region[] = {output_image_size[0], output_image_size[1], 1};
		error = clEnqueueReadImage(command_queue, cl_output_image, CL_TRUE,
		                           origin, region, 0, 0, output_image,
		                           0, NULL, NULL);
		CL_ASSERT(error == CL_SUCCESS);
#else
		size_t buffer_size = output_image_size[0]*output_image_size[1] * sizeof(*output_image);
		error = clEnqueueReadBuffer(command_queue, cl_output_image, CL_TRUE,
		                            0, buffer_size, output_image,
		                            0, NULL, NULL);
		CL_ASSERT(error == CL_SUCCESS);
#endif
	}
	
	
	// Write result to file
	{
		fprintf(stderr, "Writing %s...", output_path);
		int error = pgm_save(output_image, output_image_size[1], output_image_size[0], output_path);
		if(error)
			fputs(" error\n", stderr);
		fputs(" done\n", stderr);
	}
	
	
	// Shut down
	{
		free(output_image);
		free(source_image);
		free(blur_mask);

		cl_int error = 0;
		
		error = clReleaseKernel(kernel);
		CL_ASSERT(error == CL_SUCCESS);
		error = clReleaseProgram(program);
		CL_ASSERT(error == CL_SUCCESS);
		error = clReleaseMemObject(cl_output_image);
		CL_ASSERT(error == CL_SUCCESS);
		error = clReleaseMemObject(cl_source_image);
		CL_ASSERT(error == CL_SUCCESS);
		error = clReleaseCommandQueue(command_queue);
		CL_ASSERT(error == CL_SUCCESS);
		error = clReleaseContext(context);
		CL_ASSERT(error == CL_SUCCESS);
	}
	
	return 0;
}
