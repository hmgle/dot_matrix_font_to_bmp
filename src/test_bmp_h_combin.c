#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "dot_matrix_font_to_bmp.h"
#include "debug_log.h"

#ifndef MAX_LINE
#define MAX_LINE	1024
#endif

#define GB2312_HZK	"gb2312.hzk"
#define FONTHEIGHT	16

int main(int argc, char **argv)
{
	int opt;
	int font_fd;
	struct stat fd_stat;
	uint32_t bits_per_pix = 16;
	uint8_t *addr_fd_in;
	uint8_t gb2312buf[MAX_LINE * 2];
	uint32_t offset;
	bmp_file_t bmp;
	bmp_file_t bmp2;
	bmp_file_t bmp_all;
	uint32_t image_size;
	int ret;
	char *pret;

	while ((opt = getopt(argc, argv, "d:")) != -1) {
		switch (opt) {
		case 'd':
			bits_per_pix = atoi(optarg);
			break;
		default: /* '?' */
			fprintf(stderr,
				"Usage: %s [-d bitdepth(16 or 24)]\n",
				argv[0]);
			exit(1);
		}
	}
	pret = fgets((char *)gb2312buf, sizeof(gb2312buf) - 1, stdin);

	font_fd = open(GB2312_HZK, O_RDONLY);
	if (font_fd < 0) {
		perror("open");
		exit(1);
	}
	ret = fstat(font_fd, &fd_stat);
	if (ret == -1) {
		perror("fstat");
		exit(1);
	}
	addr_fd_in = mmap(NULL, (size_t) fd_stat.st_size,
			  PROT_READ, MAP_PRIVATE, font_fd, (off_t) 0);
	if (addr_fd_in == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	/* debug_print("gb2312buf[0] = %#x", ((uint16_t *)gb2312buf)[0]); */
	offset = gb2312code_to_fontoffset(((uint16_t *)gb2312buf)[0],
			FONTHEIGHT);
	/* debug_print("offset = %#x", offset); */
	memset(&bmp, 0, sizeof(bmp));
	set_header(&bmp, 16, 16, bits_per_pix);
	image_size = bmp.dib_h.image_size;
	bmp.pdata = malloc(image_size);
	memset(bmp.pdata, 0, image_size);
	fontdata2bmp(addr_fd_in + offset % fd_stat.st_size, 16, 16, &bmp, bits_per_pix, 0);

	/* debug_print("gb2312buf[0] = %#x", ((uint16_t *)gb2312buf)[1]); */
	offset = gb2312code_to_fontoffset(((uint16_t *)gb2312buf)[1],
					FONTHEIGHT);
	/* debug_print("offset = %#x", offset); */
	memset(&bmp2, 0, sizeof(bmp2));
	set_header(&bmp2, 16, 16, bits_per_pix);
	image_size = bmp2.dib_h.image_size;
	bmp2.pdata = malloc(image_size);
	memset(bmp2.pdata, 0, image_size);
	fontdata2bmp(addr_fd_in + offset % fd_stat.st_size, 16, 16, &bmp2, bits_per_pix, 0);

	/* debug_print("will memset bmp_all"); */

	memset(&bmp_all, 0, sizeof(bmp_all));
	set_header(&bmp_all,
		   bmp.dib_h.width + bmp2.dib_h.width,
		   bmp.dib_h.height,
		   bmp.dib_h.bits_per_pix);
	bmp_all.pdata = malloc(bmp_all.dib_h.image_size);
	/* debug_print("bmp_all.pdata is %#x", bmp_all.pdata); */
	bmp_h_combin(&bmp, &bmp2, &bmp_all);

	ret = fwrite(&bmp.bmp_h, sizeof(bmp_file_header_t), 1, stdout);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	ret = fwrite(&bmp.dib_h, sizeof(dib_header_t), 1, stdout);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	ret = fwrite(bmp.pdata, sizeof(uint8_t), image_size, stdout);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}

	if (bmp.pdata) {
		free(bmp.pdata);
		bmp.pdata = NULL;
	}


#if 0
	ret = fwrite(&bmp2.bmp_h, sizeof(bmp_file_header_t), 1, stderr);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	ret = fwrite(&bmp2.dib_h, sizeof(dib_header_t), 1, stderr);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	ret = fwrite(bmp2.pdata, sizeof(uint8_t), image_size, stderr);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
#else
	ret = fwrite(&bmp_all.bmp_h, sizeof(bmp_file_header_t), 1, stderr);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	ret = fwrite(&bmp_all.dib_h, sizeof(dib_header_t), 1, stderr);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	ret = fwrite(bmp_all.pdata, sizeof(uint8_t), bmp_all.dib_h.image_size,
			stderr);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
#endif

	if (bmp2.pdata) {
		free(bmp2.pdata);
		bmp2.pdata = NULL;
	}


	ret = munmap(addr_fd_in, (size_t) fd_stat.st_size);
	if (ret == -1) {
		perror("munmap");
		exit(1);
	}
	close(font_fd);
	return 0;
}
