#ifndef _ENCODING_CONVERT_H
#define _ENCODING_CONVERT_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifndef MAX_LINE
#define MAX_LINE	1024
#endif

int get_utf8_length(const uint8_t *src);
int utf8tounicode(const uint8_t *src, uint8_t *dst);
uint16_t unicode_to_gb2312(uint16_t unicode, const uint16_t *mem_gb2312, int gb2312_num);
uint16_t *mem_gb2312(const char *gb2312filename, int *gb2312_num);
void unmem_gb2312(uint16_t *add);

#endif
