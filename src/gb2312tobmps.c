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
#define ASCII_HZK	"ASC16"

int main(int argc, char **argv)
{
	int opt;
	int font_fd;
	struct stat fd_stat;
	int ascii_fd;
	struct stat ascii_fd_stat;
	uint32_t bits_per_pix = 16;
	uint8_t *addr_fd_in;
	uint8_t *addr_ascii_fd_in;
	uint8_t gb2312buf[MAX_LINE * 2];
	uint32_t offset;
	bmp_file_t bmp;
	uint32_t image_size;
	int ret;
	char *pret;
	int color_anti_flag = 0;
	int i;

	while ((opt = getopt(argc, argv, "d:c")) != -1) {
		switch (opt) {
		case 'd':
			bits_per_pix = atoi(optarg);
			break;
		case 'c':
			color_anti_flag = 1;
			break;
		default: /* '?' */
			fprintf(stderr, 
				"Usage: %s [-d bitdepth(16 or 24)] "
				"[-c]\n", 
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

	ascii_fd = open(ASCII_HZK, O_RDONLY);
	if (ascii_fd < 0) {
		perror("open");
		exit(1);
	}
	ret = fstat(ascii_fd, &ascii_fd_stat);
	if (ret == -1) {
		perror("fstat");
		exit(1);
	}
	addr_ascii_fd_in = mmap(NULL, (size_t) ascii_fd_stat.st_size,
			  PROT_READ, MAP_PRIVATE, ascii_fd, (off_t) 0);
	if (addr_ascii_fd_in == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	i = 0;
	for (;;) {
		memset(&bmp, 0, sizeof(bmp));
		set_header(&bmp, 16, 16, bits_per_pix);
		image_size = bmp.dib_h.image_size;
		bmp.pdata = malloc(image_size);
		memset(bmp.pdata, 0, image_size);

		if (gb2312buf[i] > 0xA0 && gb2312buf[i]  < 0xff) {
			offset = gb2312code_to_fontoffset(gb2312buf[i] + 0x100 * gb2312buf[i + 1]);
			i += 2;
			fontdata2bmp(addr_fd_in + offset, 16, 16, &bmp, bits_per_pix, color_anti_flag);
		} else if (gb2312buf[i] > 0x1f && gb2312buf[i] < 0x80) { /* ascii */
			offset = ascii_to_fontoffset(gb2312buf[i]);
			debug_print("offset = %#x", offset);
			fontdata2bmp(addr_ascii_fd_in + offset, 8, 16, &bmp, bits_per_pix, color_anti_flag);
			i++;
		} else
			break;

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
	}

	if (bmp.pdata) {
		free(bmp.pdata);
		bmp.pdata = NULL;
	}
	ret = munmap(addr_fd_in, (size_t) fd_stat.st_size);
	if (ret == -1) {
		perror("munmap");
		exit(1);
	}
	close(font_fd);
	return 0;
}
