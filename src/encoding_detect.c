#include "encoding_detect.h"
#include "encoding_convert.h"

int
detect_file_encoding(FILE *text_fp)
{
	int ret;
	fpos_t pos;
	char buf[MAX_LINE];
	size_t len;
	enum file_encoding_type encoding_type;

	ret = fgetpos(text_fp, &pos);
	assert(!ret);

	rewind(text_fp);
	if (fgets(buf, sizeof(buf), text_fp) == NULL)
		return -1;
	len = strlen(buf);
	if (len >= 3 && buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf) {
		encoding_type = UTF8_WITH_BOM;
		goto end;
	}
	else {
		ret = is_utf8((const uint8_t *)buf, len);
		if (ret > 0) {
			encoding_type = GBK;
			goto end;
		} else if (ret == -1) {
			encoding_type = UTF8_NO_BOM;
			goto end;
		}
	}

	while (fgets(buf, sizeof(buf), text_fp)) {
		len = strlen(buf);
		ret = is_utf8((const uint8_t *)buf, len);
		if (ret > 0) {
			encoding_type = GBK;
			goto end;
		} else if (ret == -1) {
			encoding_type = UTF8_NO_BOM;
			goto end;
		}
	}
	encoding_type = UTF8_NO_BOM;
end:
	ret = fsetpos(text_fp, &pos);
	assert(!ret);
	return encoding_type;
}
