#include <stdio.h>
#include "dot_matrix_font_to_bmp.h"
#include "bmp_io.h"
#include "debug_log.h"

int main(int argc, char **argv)
{
	bmp_file_t bmp;
	uint32_t image_size;

	set_header(&bmp, 16, 16, 16);
	image_size = bmp.dib_h.image_size;
	bmp.pdata = malloc(image_size);
	memset(bmp.pdata, 0, image_size);

	output_bmp(stdout, &bmp);
	return 0;
}
