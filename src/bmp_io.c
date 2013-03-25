#include <stdlib.h>
#include "debug_log.h"
#include "bmp_io.h"

/* 读取成功返回0, 该函数申请内存但不释放， 调用完后记得释放 */
int
read_and_alloc_one_bmp(FILE *fp, bmp_file_t *ptrbmp)
{
	size_t numread;

	numread = fread(&ptrbmp->bmp_h, sizeof(struct bmp_file_header), 1, fp);
	if (!numread)
		return -1;
	numread = fread(&ptrbmp->dib_h, sizeof(struct dib_header), 1, fp);
	if (!numread)
		return -1;

	assert(ptrbmp->dib_h.image_size > 0);
	ptrbmp->pdata = malloc(ptrbmp->dib_h.image_size);
	numread = fread(ptrbmp->pdata, 1, ptrbmp->dib_h.image_size, fp);
	if (!numread)
		return -1;
	return 0;
}

void
free_bmp(bmp_file_t *ptrbmp)
{
	if (ptrbmp) {
		if (ptrbmp->pdata) {
			free(ptrbmp->pdata);
			ptrbmp->pdata = NULL;
		}
		free(ptrbmp);
		ptrbmp = NULL;
	}
}

int
output_bmp(FILE *fp, bmp_file_t *ptrbmp)
{
	int ret;

	ret = fwrite(&ptrbmp->bmp_h, 
		     sizeof(struct bmp_file_header),
		     1,
		     fp);
	if (ret < 0) {
		debug_print("fwrite fail");
		return -1;
	}

	ret = fwrite(&ptrbmp->dib_h, 
		     sizeof(struct dib_header),
		     1,
		     fp);
	if (ret < 0) {
		debug_print("fwrite fail");
		return -1;
	}

	ret = fwrite(ptrbmp->pdata, 
		     1, 
		     ptrbmp->dib_h.image_size, 
		     fp);
	if (ret < 0) {
		debug_print("fwrite fail");
		return -1;
	}
	return 0;
}

