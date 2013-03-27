#include "encoding_detect.h"

int main(int argc, char **argv)
{
	FILE *fp;
	int en;

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "fopen fail\n");
		return -1;
	}

	en = detect_file_encoding(fp);
	fprintf(stderr, "en is %d\n", en);

	fclose(fp);
	return 0;
}
