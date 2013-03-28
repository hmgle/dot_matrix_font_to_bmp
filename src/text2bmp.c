#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "dot_matrix_font_to_bmp.h"
#include "encoding_convert.h"
#include "encoding_detect.h"
#include "bmp_io.h"
#include "debug_log.h"

#define GB2312_HZK	"gb2312.hzk"
#define ASCII_HZK	"ASC16"
#define FONT_BMP_SIZE	1024

struct text_style {
	uint32_t left_margin;
	uint32_t right_margin;
	uint32_t up_margin;
	uint32_t down_margin;
	uint32_t line_spacing;
	uint32_t character_spacing;
	uint32_t max_line_length;
};

static void show_usage(const char *pro_name);

static void show_usage(const char *pro_name)
{
	fprintf(stderr, "Usage: %s [Options] [inputfile]\n", pro_name);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "        -l val        set left margin\n");
	fprintf(stderr, "        -r val        set right margin\n");
	fprintf(stderr, "        -u val        set top margin\n");
	fprintf(stderr, "        -d val        set bottom margin\n");
	fprintf(stderr, "        -i val        set line spacing\n");
	fprintf(stderr, "        -c val        set character spacing\n");
	fprintf(stderr, "        -m val        set max line length\n");
	fprintf(stderr, "        -b val        set bits_per_pix\n");
	fprintf(stderr, "        -o            anti_color\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	int opt;
	struct text_style style;
	uint32_t bits_per_pix = 16;
	int color_anti_flag = 0;
	FILE *in = stdin;
	int gb2312_num;
	uint16_t *mem_addr;
	uint8_t linebuf[MAX_LINE];
	uint8_t gb2312buf[MAX_LINE * 2];
	uint8_t *ptr;
	uint8_t *ptr_gb2312;
	uint8_t unicode[2] = {0};
	uint16_t gb2312_code;
	uint8_t *addr_fd_in;
	struct stat fd_stat;
	int font_fd;
	int ascii_fd;
	struct stat ascii_fd_stat;
	uint8_t *addr_ascii_fd_in;
	int once_read;
	int i;
	uint32_t offset;
	bmp_file_t bmp_char;
	bmp_file_t bmp_line;
	bmp_file_t bmp_all;
	bmp_file_t bmp_blank;
	int encoding_type = UTF8_NO_BOM;
	int ret;

	memset(&style, 0, sizeof(struct text_style));
	while ((opt = getopt(argc, argv, "l:r:u:d:i:c:m:b:oh?")) != -1) {
		switch (opt) {
		case 'l': /* 左边距 */
			style.left_margin = strtol(optarg, NULL, 0);
			break;
		case 'r': /* 右边距 */
			style.right_margin = strtol(optarg, NULL, 0);
			break;
		case 'u': /* 上边距 */
			style.up_margin = strtol(optarg, NULL, 0);
			break;
		case 'd': /* 下边距 */
			style.down_margin = strtol(optarg, NULL, 0);
			break;
		case 'i': /* 行间距 */
			style.line_spacing = strtol(optarg, NULL, 0);
			break;
		case 'c': /* 字符间距 */
			style.character_spacing = strtol(optarg, NULL, 0);
			break;
		case 'm': /* 每行最大长度 */
			style.max_line_length = strtol(optarg, NULL, 0);
			break;
		case 'b': /* 位深 */
			bits_per_pix = strtol(optarg, NULL, 0);
			break;
		case 'o': /* 白底黑字 */
			color_anti_flag = 1;
			break;
		default:
			show_usage(argv[0]);
			exit(1);
		}
	} /* while ((opt = getopt(argc, argv, "l:r:u:d:i:c:m:b:o:")) != -1) */
	if (argc > optind) {
		in = fopen(argv[optind], "r");
		if (!in) {
			fprintf(stderr,
				"Unable to open input file \"%s\": %s\n",
				argv[optind], strerror(errno));
			return 1;
		}
		encoding_type = detect_file_encoding(in);
	}

	mem_addr = mem_gb2312("./GB2312", &gb2312_num);

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

	memset(&bmp_char, 0, sizeof(bmp_char));
	bmp_char.pdata = malloc(FONT_BMP_SIZE);
	memset(&bmp_line, 0, sizeof(bmp_line));
	memset(&bmp_all, 0, sizeof(bmp_all));
	memset(&bmp_blank, 0, sizeof(bmp_blank));
	bmp_blank.pdata = malloc(FONT_BMP_SIZE);

	once_read = (style.max_line_length > 0) 
			? (style.max_line_length + 1) : (sizeof(linebuf) - 1);
	if (encoding_type == UTF8_WITH_BOM) {
		if (fseek(in, 3, SEEK_SET)) {
			debug_print("fseek failed: %s", strerror(errno));
			exit(errno);
		}
	}
	for(;;) {
		if (encoding_type == UTF8_NO_BOM || encoding_type == UTF8_WITH_BOM) {
			if (fgets_utf8((char *)linebuf, once_read, in) == NULL)
				break;
			ptr_gb2312 = gb2312buf;
			ptr = linebuf;
			if (*ptr == '\n')
				*ptr = ' ';
			while (*ptr) {
				ret = utf8tounicode(ptr, unicode);
				if (ret < 0) {
					debug_print("utf8tounicode return %d\n", ret);
					exit(1);
				}
				ptr += ret;
				gb2312_code = unicode_to_gb2312(unicode[0] + unicode[1] * 0x100, 
								mem_addr, 
								gb2312_num);
				ptr_gb2312[0] = gb2312_code % 0x100;
				if (gb2312_code / 0x100 > 0) {
					ptr_gb2312[1] = gb2312_code / 0x100;
					ptr_gb2312 += 2;
				} else
					ptr_gb2312 += 1;
			}
			ptr_gb2312[0] = '\0';
		} else if (encoding_type == GBK) {

		}

		/*
		 * gb2312tobmps
		 */
		i = 0;
		for (;;) {
			if (gb2312buf[i] > 0xA0 && gb2312buf[i]  < 0xff) {
				offset = gb2312code_to_fontoffset(gb2312buf[i] + 0x100 * gb2312buf[i + 1]);
				i += 2;
				set_header(&bmp_char, 16, 16, bits_per_pix);
				memset(bmp_char.pdata, 0, bmp_char.dib_h.image_size);
				fontdata2bmp(addr_fd_in + offset, 16, 16, &bmp_char, bits_per_pix, color_anti_flag);

			} else if (gb2312buf[i] > 0x1f && gb2312buf[i] < 0x80) {
				offset = ascii_to_fontoffset(gb2312buf[i]);
				i++;
				set_header(&bmp_char, 8, 16, bits_per_pix);
				memset(bmp_char.pdata, 0, bmp_char.dib_h.image_size);
				fontdata2bmp(addr_ascii_fd_in + offset, 8, 16, &bmp_char, bits_per_pix, color_anti_flag);
			} else if (gb2312buf[i] == '\t') {
				i++;
				create_blank_bmp(&bmp_char,
						 8 * 8,
						 1,
						 bits_per_pix,
						 color_anti_flag);
			} else
				break;

			bmp_h_combin_3(&bmp_line, &bmp_char, color_anti_flag);
			if (style.character_spacing > 0) {
				create_blank_bmp(&bmp_blank, 
						style.character_spacing, 
						1, 
						bits_per_pix, 
						color_anti_flag);
				bmp_h_combin_3(&bmp_line, &bmp_blank, color_anti_flag);
			}
		} /* for (;;) */
		bmp_v_combin_3(&bmp_all, &bmp_line, color_anti_flag);
		if (style.line_spacing > 0) {
			create_blank_bmp(&bmp_blank, 
					1, 
					style.line_spacing, 
					bits_per_pix, 
					color_anti_flag);
			bmp_v_combin_3(&bmp_all, &bmp_blank, color_anti_flag);
		}
		if (bmp_line.pdata) {
			free(bmp_line.pdata);
			bmp_line.pdata = NULL;
		}
		memset(&bmp_line, 0, sizeof(bmp_line));
	} /* while (fgets_utf8((char *)linebuf, sizeof(linebuf) - 1, in)) */

	/*
	 * 格式处理
	 */
	if (style.left_margin > 0) {
		create_blank_bmp(&bmp_blank, 
				style.left_margin, 
				1, 
				bits_per_pix, 
				color_anti_flag);
		bmp_h_combin_3(&bmp_blank, &bmp_all, color_anti_flag);
		memcpy(&bmp_all.bmp_h, &bmp_blank.bmp_h, sizeof(bmp_file_header_t));
		memcpy(&bmp_all.dib_h, &bmp_blank.dib_h, sizeof(dib_header_t));
		bmp_all.pdata = realloc(bmp_all.pdata, bmp_all.dib_h.image_size);
		memcpy(bmp_all.pdata, bmp_blank.pdata, bmp_all.dib_h.image_size);
	}
	if (style.right_margin > 0) {
		create_blank_bmp(&bmp_blank, 
				style.right_margin, 
				1, 
				bits_per_pix, 
				color_anti_flag);
		bmp_h_combin_3(&bmp_all, &bmp_blank, color_anti_flag);
	}
	if (style.up_margin > 0) {
		create_blank_bmp(&bmp_blank, 
				1, 
				style.up_margin, 
				bits_per_pix, 
				color_anti_flag);
		bmp_v_combin_3(&bmp_blank, &bmp_all, color_anti_flag);
		memcpy(&bmp_all.bmp_h, &bmp_blank.bmp_h, sizeof(bmp_file_header_t));
		memcpy(&bmp_all.dib_h, &bmp_blank.dib_h, sizeof(dib_header_t));
		bmp_all.pdata = realloc(bmp_all.pdata, bmp_all.dib_h.image_size);
		memcpy(bmp_all.pdata, bmp_blank.pdata, bmp_all.dib_h.image_size);
	}
	if (style.down_margin > 0) {
		create_blank_bmp(&bmp_blank, 
				1, 
				style.down_margin, 
				bits_per_pix, 
				color_anti_flag);
		bmp_v_combin_3(&bmp_all, &bmp_blank, color_anti_flag);
	}

	/*
	 * output
	 */
	output_bmp(stdout, &bmp_all);

	/*
	 * release
	 */
	if (bmp_blank.pdata) {
		free(bmp_blank.pdata);
		bmp_blank.pdata = NULL;
	}
	free(bmp_char.pdata);
	if (bmp_all.pdata) {
		free(bmp_all.pdata);
		bmp_all.pdata = NULL;
	}
	if (bmp_line.pdata) {
		free(bmp_line.pdata);
		bmp_line.pdata = NULL;
	}
	ret = munmap(addr_ascii_fd_in, (size_t) ascii_fd_stat.st_size);
	if (ret == -1) {
		perror("munmap");
		exit(1);
	}
	ret = munmap(addr_fd_in, (size_t) fd_stat.st_size);
	if (ret == -1) {
		perror("munmap");
		exit(1);
	}
	close(ascii_fd);
	close(font_fd);
	unmem_gb2312(mem_addr);
	if (in != stdin)
		fclose(in);
	return 0;
}
