#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "dot_matrix_font_to_bmp.h"
#include "debug_log.h"

struct text_style {
	uint32_t left_margin;
	uint32_t right_margin;
	uint32_t up_margin;
	uint32_t down_margin;
	uint32_t line_spacing;
	uint32_t character_spacing;
	uint32_t max_line_length;
};

int main(int argc, char **argv)
{
	int opt;
	struct text_style style = {0};
	FILE *in = stdin;

	while ((opt = getopt(argc, argv, "l:r:u:d:i:c:m:")) != -1) {
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
		default:
			fprintf(stderr, "Usage: %s [-l left_margin] "
					"[-r right_margin] "
					"[-u up_margin] "
					"[-d down_margin] "
					"[-i line_spacing] "
					"[-c character_spacing] "
					"[-m max_line_length] "
					"[inputfile]\n", argv[0]);
			exit(1);
		}
	}
	if (argc > optind) {
		in = fopen(argv[optind], "r");
		if (!in) {
			fprintf(stderr,
				"Unable to open input file \"%s\": %s\n",
				argv[optind], strerror(errno));
			return 1;
		}
	}

	if (in != stdin)
		fclose(in);
	return 0;
}
