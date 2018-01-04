__kernel
void
blur(__global   unsigned char* source,
                int source_w,
                int source_h,
     __global   unsigned char* target,
                int target_w,
                int target_h,
     __constant float* mask,
                int mask_radius)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	if(0 <= x && x < source_w
	&& 0 <= y && y < source_h)
	{
		int   mask_side = 2*mask_radius + 1;
		int   n         = 0;
		float c         = 0;
		
		for(int row = 0; row < mask_side; ++row)
		{
			for(int col = 0; col < mask_side; ++col)
			{
				int2 p = {x + col - mask_radius, y + row - mask_radius};
				
				if(0 <= p.x && p.x < source_w
				&& 0 <= p.y && p.y < source_h)
				{
					if(mask[row*mask_side + col] > 0)
					{
						c += source[p.y*source_w + p.x] * mask[row*mask_side + col];
						n += 1;
					}
				}
			}
		}
		
		if(n > 0)
		{
			uint          color = c / n;
			unsigned char pixel = (unsigned char)color;
			target[y*source_w + x] = pixel;
		}
	}
}

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;


__kernel
void
blur_image( __read_only image2d_t source,
           __write_only image2d_t target,
           __constant   float*    mask,
                        int       mask_radius)
{
	int  x           = get_global_id(0);
	int  y           = get_global_id(1);
	int2 source_size = get_image_dim(source);
	
	if(0 <= x && x < source_size.x
	&& 0 <= y && y < source_size.y)
	{
		int   mask_side = 2*mask_radius + 1;
		int   n         = 0;
		float c         = 0;
		
		for(int row = 0; row < mask_side; ++row)
		{
			for(int col = 0; col < mask_side; ++col)
			{
				if(mask[row*mask_side + col] > 0)
				{
					int2 p = {x + col - mask_radius, y + row - mask_radius};
					
					if(0 <= p.x && p.x < source_size.x
					&& 0 <= p.y && p.y < source_size.y)
					{
						c += read_imageui(source, sampler, p).x * mask[row*mask_side + col];
						n += 1;
					}
				}
			}
		}
		
		if(n > 0)
		{
			uint  color = c / n;
			uint4 pixel = {color, color, color, color};
			write_imageui(target, (int2)(x, y), pixel);
		}
	}
}
