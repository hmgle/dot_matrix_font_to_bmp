#include <unistd.h>
#include <stdlib.h>
#include "dot_matrix_font_to_bmp.h"
#include "debug_log.h"

int main(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "l:r:u:d:i:c:")) != -1) {
		switch (opt) {
		case 'l': /* 左边距 */
			break;
		case 'r': /* 右边距 */
			break;
		case 'u': /* 上边距 */
			break;
		case 'd': /* 下边距 */
			break;
		case 'i': /* 行间距 */
			break;
		case 'c': /* 字符间距 */
			break;
		default:
			fprintf(stderr, "Usage: %s [-l left_margin] "
					"[-r right_margin] "
					"[-u up_margin] "
					"[-d down_margin] "
					"[-i line_spacing] "
					"[-c character_spacing]\n", argv[0]);
			exit(1);
		}
	}

	return 0;
}
