#ifndef _ENCODING_DETECT_H
#define _ENCODING_DETECT_H

#include "debug_log.h"
#include <stdio.h>
#include <stdint.h>

#ifndef MAX_LINE
#define MAX_LINE	1024
#endif

enum file_encoding_type {
	UTF8_NO_BOM,
	UTF8_WITH_BOM,
	GBK,
};

int detect_file_encoding(FILE *text_fp);

#endif /* #ifndef _ENCODING_DETECT_H */
