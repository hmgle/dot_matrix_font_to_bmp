#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bmp_io.h"
#include "debug_log.h"

#ifndef	MAXLINE
#define	MAXLINE		1024
#endif

int main(int argc, char **argv)
{
	bmp_file_t bmp_all;
	bmp_file_t bmp_1;
	char ch;
	int ret;

	memset(&bmp_all, 0, sizeof(bmp_all));
	memset(&bmp_1, 0, sizeof(bmp_1));
	ret = read_and_alloc_one_bmp(stdin, &bmp_all);
	while (!read_and_alloc_one_bmp(stdin, &bmp_1)) {
		bmp_v_combin_3(&bmp_all, &bmp_1);
		free(bmp_1.pdata);

		if ((ch = fgetc(stdin)) == 'B')
			ungetc(ch, stdin);
		else /* 后面没有位图流了 */
			break;
	}

	ret = output_bmp(stdout, &bmp_all);
	free(bmp_all.pdata);
	return 0;
}
