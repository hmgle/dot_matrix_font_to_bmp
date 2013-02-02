#include "encoding_convert.h"
#include "debug_log.h"

#ifndef GB2312_MEM_SIZE
#define GB2312_MEM_SIZE		(7580 * 4)
#endif

static inline uint8_t hex_ch_to_val(char hex_ch);

int 
get_utf8_length(const uint8_t *src)
{
	int length;

	switch (*src) {
	case 0x0 ... 0x7f:
		length = 1;
		break;
	case 0xC0 ... 0xDF:
		length = 2;
		break;
	case 0xE0 ... 0xEF:
		length = 3;
		break;
	case 0xF0 ... 0xF7:
		length = 4;
		break;
	default:
		length = -1;
	}
	return length;
}

int
utf8tounicode(const uint8_t *src, uint8_t *dst)
{
	int length;
	uint8_t unicode[2] = {0}; /* 小端序 */

	length = get_utf8_length(src);
	if (length < 0)
		return -1;
	switch (length) {
	case 1:
		*dst = *src;
		*(dst + 1) = 0; /* 经测试，若不加该句，dst+1里面的值会变成0x5b */
		return 1;
		break;
	case 2:
		unicode[0] = *(src + 1) & 0x3f;
		unicode[0] += (*src & 0x3) << 6;
		unicode[1] = (*src & 0x7 << 2) >> 2;
		break;
	case 3:
		unicode[0] = *(src + 2) & 0x3f;
		unicode[0] += (*(src + 1) & 0x3) << 6;
		unicode[1] = (*(src + 1) & 0xF << 2) >> 2;
		unicode[1] += (*src & 0xf) << 4;
		break;
	case 4:
		/* not support now */
		return -1;
	}
	*dst = unicode[0];
	*(dst + 1) = unicode[1];
	return length;
}

uint16_t
unicode_to_gb2312(uint16_t unicode, const uint16_t *mem_gb2312, int gb2312_num)
{
	int i;

#if 1
	for (i = 0; i < gb2312_num; i++)
		if (mem_gb2312[2 * i] == unicode)
			return mem_gb2312[2 * i + 1];
	return -1;
#else /* 二分法 */
	if (gb2312_num <= 0)
		return -1;
	i = gb2312_num / 2;
	if (mem_gb2312[2 * i] == unicode)
		return mem_gb2312[2 * i + 1];
	else if (mem_gb2312[2 * i] < unicode)
		return unicode_to_gb2312(unicode, 
					 &mem_gb2312[2 * i + 2], 
					 gb2312_num - i - 1);
	else /* mem_gb2312[2 * i] > unicode */
		return unicode_to_gb2312(unicode, 
					 mem_gb2312, 
					 i);
#endif
}

static inline uint8_t 
hex_ch_to_val(char hex_ch)
{
	if (hex_ch >= '0' && hex_ch <= '9')
		return hex_ch - '0';
	else if (hex_ch >= 'A' && hex_ch <= 'F')
		return hex_ch - 'A' + 10;
	else if (hex_ch >= 'a' && hex_ch <= 'f')
		return hex_ch - 'a' + 10;
	return -1;
}

uint16_t *
mem_gb2312(const char *gb2312filename, int *gb2312_num)
{
	FILE *gb2312_fp;
	uint16_t *ptrmem;
	char *ptrch;
	char buf[MAX_LINE];
	int i;

	gb2312_fp = fopen(gb2312filename, "r");
	if (gb2312_fp == NULL) {
		perror("fopen");
		exit(1);
	}
	ptrmem = malloc(GB2312_MEM_SIZE);
	if (!ptrmem) {
		perror("malloc");
		exit(1);
	}
	memset(ptrmem, 0, GB2312_MEM_SIZE);
	i = 0;
	while (fgets(buf, MAX_LINE, gb2312_fp) != NULL) {
		if (strstr(buf, "/x") == NULL)
			continue;

		/* unicode */
		ptrch = strchr(buf, 'U');
		ptrch++;
		*(ptrmem + i * 2) =  hex_ch_to_val(ptrch[0]) * 0x1000
				    + hex_ch_to_val(ptrch[1]) * 0x100
				    + hex_ch_to_val(ptrch[2]) * 0x10 
				    + hex_ch_to_val(ptrch[3]);

		/* gb2312 */
		ptrch = strstr(ptrch, "/x");
		ptrch += 2;
		if (ptrch[2] != '/') { /* 单字节 */
			*(ptrmem + i * 2 + 1) = hex_ch_to_val(ptrch[1])
						+ hex_ch_to_val(ptrch[0]) * 0x10;
		} else { /* 两个字节 */
			*(ptrmem + i * 2 + 1) = hex_ch_to_val(ptrch[5]) * 0x100
						+ hex_ch_to_val(ptrch[4]) * 0x1000
						+ hex_ch_to_val(ptrch[1])
						+ hex_ch_to_val(ptrch[0]) * 0x10;
		}
		i++;
	} /* i should be 7573 */
	*gb2312_num = i;

	fclose(gb2312_fp);
	return ptrmem;
}

void
unmem_gb2312(uint16_t *add)
{
	if (add) free(add);
}
