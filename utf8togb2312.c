#include "encoding_convert.h"
#include "debug_log.h"

int main(int argc, char **argv)
{
	uint8_t linebuf[MAX_LINE];
	uint8_t gb2312buf[MAX_LINE * 2];
	uint8_t *ptr;
	uint8_t *ptr_gb2312;
	uint8_t unicode[2] = {0};
	uint16_t *mem_addr;
	uint16_t gb2312_code;
	int gb2312_num;
	int ret;
	char *pret;

	mem_addr = mem_gb2312("./GB2312", &gb2312_num);

	/* ret = scanf("%1023s\n", linebuf); */
	pret = fgets((char *)linebuf, sizeof(linebuf) - 1, stdin);
	debug_print("%s", linebuf);
	ptr = linebuf;
	ptr_gb2312 = gb2312buf;
	while (*ptr) {
		ret = utf8tounicode(ptr, unicode);
		if (ret < 0) {
			debug_print("utf8tounicode return %d\n", ret);
			exit(1);
		}
		ptr += ret;
		debug_print("u[0] = %#x u[1] = %#x", unicode[0], unicode[1]);
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
	debug_print("%s", gb2312buf);
	fwrite(gb2312buf, 1, strlen((char *)gb2312buf), stdout);

	unmem_gb2312(mem_addr);
	return 0;
}
