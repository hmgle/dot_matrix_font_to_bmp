/*
 * for example: 
 * 汉字“度”的gb2312 EUC-CN 编码为0xB6C8, 
 * 16*16字库gb2312.hzk, 要生成“度”的24位的位图，输入：
 * ./test ./gb2312.hzk 0xb6c8 ./b6c8.bmp 24
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "dot_matrix_font_to_bmp.h"

void show_usage(char *thisname);

void show_usage(char *thisname)
{
	printf("usage: %s <font_repo> <addr> <outbmp> <bit_per_pix>\n", thisname);
}

int main(int argc, char **argv)
{
	int font_fd;
	FILE *save_fp;
	struct stat fd_stat;
	uint8_t *addr_fd_in;
	uint32_t offset;
	int ret;
	uint32_t bits_per_pix;
	uint32_t image_size;
	bmp_file_t bmp;

	if (argc < 5) {
		show_usage(argv[0]);
		exit(1);
	}
	font_fd = open(argv[1], O_RDONLY);
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
	offset = strtol(argv[2], NULL, 0);
	offset = ((offset / 0x100 - 0xA1) * 94) + (offset % 0x100 - 0xA1);
	offset *= 32;
	printf("offset = %d\n", offset);
	if ((save_fp = fopen(argv[3], "wb")) == NULL) {
		perror("fopen");
		exit(1);
	}
	bits_per_pix = atoi(argv[4]);
	memset(&bmp, 0, sizeof(bmp));
	set_header(&bmp, 16, 16, bits_per_pix);
	image_size = bmp.dib_h.image_size;
	bmp.pdata = malloc(image_size);
	memset(bmp.pdata, 0, image_size);
	fontdata2bmp(addr_fd_in + offset, 16, 16, &bmp, bits_per_pix, 0);
	ret = fwrite(&bmp.bmp_h, sizeof(bmp_file_header_t), 1, save_fp);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	ret = fwrite(&bmp.dib_h, sizeof(dib_header_t), 1, save_fp);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	ret = fwrite(bmp.pdata, sizeof(uint8_t), image_size, save_fp);
	if (ret < 0) {
		perror("fwrite");
		exit(1);
	}
	fflush(save_fp);

	if (bmp.pdata) {
		free(bmp.pdata);
		bmp.pdata = NULL;
	}
	fclose(save_fp);
	ret = munmap(addr_fd_in, (size_t) fd_stat.st_size);
	if (ret == -1) {
		perror("munmap");
		exit(1);
	}
	close(font_fd);
	return 0;
}
