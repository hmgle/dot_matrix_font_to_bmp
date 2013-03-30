#include <stdio.h>
#include "debug_log.h"
#include "dot_matrix_font_to_bmp.h"

void 
set_header(bmp_file_t *pbmp_f, 
	   uint32_t width, 
	   uint32_t height, 
	   uint16_t bits_per_pix)
{
	uint32_t rowsize;

	pbmp_f->bmp_h.magic[0] = 'B';
	pbmp_f->bmp_h.magic[1] = 'M';
	pbmp_f->bmp_h.offset = sizeof(bmp_file_header_t) + sizeof(dib_header_t);
	pbmp_f->dib_h.dib_header_size = sizeof(dib_header_t);
	pbmp_f->dib_h.width = width;
	pbmp_f->dib_h.height = height;
	pbmp_f->dib_h.planes = 1;
	pbmp_f->dib_h.bits_per_pix = bits_per_pix;
	pbmp_f->dib_h.compression = 0;
	rowsize = (bits_per_pix * width + 31) / 32 * 4; /* 4字节对齐 */
	pbmp_f->dib_h.image_size = rowsize * height;
	pbmp_f->dib_h.x_pix_per_meter = 0;
	pbmp_f->dib_h.y_pix_per_meter = 0;
	pbmp_f->dib_h.colors_in_colortable = 0;
	pbmp_f->dib_h.important_color_count = 0;
	pbmp_f->bmp_h.file_size = pbmp_f->bmp_h.offset + pbmp_f->dib_h.image_size;
}

void 
get_header(const bmp_file_t *pbmp_f, bmp_file_header_t *bmp_header, dib_header_t *dib_header)
{
	bmp_header->magic[0] = pbmp_f->bmp_h.magic[0];
	bmp_header->magic[1] = pbmp_f->bmp_h.magic[1];
	bmp_header->file_size = pbmp_f->bmp_h.file_size;
	bmp_header->reserved1 = pbmp_f->bmp_h.reserved1;
	bmp_header->reserved2 = pbmp_f->bmp_h.reserved2;
	bmp_header->offset = pbmp_f->bmp_h.offset;

	dib_header->dib_header_size = pbmp_f->dib_h.dib_header_size;
}

void 
conv_row(const uint8_t *ptrfontdata, 
	 uint32_t width, 
	 uint8_t *pdest,
	 uint16_t bits_per_pix,
	 int color_anti_flag)
{
	uint32_t i;
	int char_num;
	int char_bit;
	char bit;
	uint8_t *ptmp;

	ptmp = pdest;
	for (i = 0; i < width; i++) {
		char_num = i / 8;
		char_bit = 7 - i % 8;
		bit = ptrfontdata[char_num] & (1 << char_bit);
		bit = color_anti_flag ? !bit : bit;
		if (bit) {
			switch (bits_per_pix) {
			case 1:
				break;
			case 16:
				memset(ptmp, 0xff, 2);
				ptmp += 2;
				break;
			case 24:
				memset(ptmp, 0xff, 3);
				ptmp += 3;
				break;
			default:
				break;
			}
		} else {
			switch (bits_per_pix) {
			case 1:
				break;
			case 16:
				memset(ptmp, 0x0, 2);
				ptmp += 2;
				break;
			case 24:
				memset(ptmp, 0x0, 3);
				ptmp += 3;
				break;
			default:
				break;
			}
		}
	}
}

void 
fontdata2bmp(const uint8_t *ptrfontdata, 
	     uint32_t width, 
	     uint32_t height, 
	     bmp_file_t *ptrbmp, 
	     uint16_t bits_per_pix,
	     int color_anti_flag)
{
	uint32_t rowsize;
	uint8_t *ptrbmpdata;
	int i;

	ptrbmpdata = ptrbmp->pdata;
	set_header(ptrbmp, width, height, bits_per_pix);
	rowsize = (bits_per_pix * width + 31) / 32 * 4; /* 4字节对齐 */
#if 0
	for (i = 0; i < height; i++) /* 倒立的位图 */
#else
	for (i = height - 1; i >= 0; i--) /* 正立的位图 */
#endif
	{
		conv_row(ptrfontdata + (width + 7) / 8 * i, 
			 width, 
			 ptrbmpdata,
			 bits_per_pix,
			 color_anti_flag);
		ptrbmpdata += rowsize;
	}
}

uint32_t 
gb2312code_to_fontoffset(uint32_t gb2312code)
{
	uint32_t fontoffset;

	fontoffset = (gb2312code % 0x100 - 0xA1) * 94
		     + (gb2312code / 0x100 - 0xA1);
	fontoffset *= 32; /* for HZK16 */
	return fontoffset;
}

uint32_t 
ascii_to_fontoffset(uint32_t ascii)
{
	return (ascii * 16) + 1;
}

bmp_file_t *
create_blank_bmp(bmp_file_t *dst, 
		uint32_t w, uint32_t h, 
		uint16_t bits_per_pix, 
		int color_anti_flag)
{
	uint32_t rowsize;

	set_header(dst, w, h, bits_per_pix);
	rowsize = (bits_per_pix * w + 31) / 32 * 4; /* 4字节对齐 */
	if (!color_anti_flag)
		memset(dst->pdata, 0, rowsize * h);
	else
		memset(dst->pdata, 0xff, rowsize * h);

	return dst;
}

/*
 * 位图水平合并
 */
bmp_file_t *
bmp_h_combin(const bmp_file_t *src1, const bmp_file_t *src2, bmp_file_t *dst)
{
	uint32_t i;
	uint32_t rowsize;
	uint32_t rowsize_src1;
	uint32_t rowsize_src2;
	uint32_t row_length_src1;
	uint32_t row_length_src2;
	uint8_t *ptrbmpdata;

	memset(&dst->bmp_h, 0, sizeof(struct bmp_file_header));
	memset(&dst->dib_h, 0, sizeof(struct dib_header));

	dst->bmp_h.magic[0] = 'B';
	dst->bmp_h.magic[1] = 'M';
	dst->bmp_h.offset = sizeof(bmp_file_header_t) + sizeof(dib_header_t);
	dst->dib_h.dib_header_size = sizeof(dib_header_t);
	dst->dib_h.width = src1->dib_h.width + src2->dib_h.width;
	dst->dib_h.height = src1->dib_h.height;
	dst->dib_h.planes = 1;
	dst->dib_h.bits_per_pix = src1->dib_h.bits_per_pix;
	dst->dib_h.compression = 0;

	rowsize = (dst->dib_h.bits_per_pix * dst->dib_h.width + 31) / 32 * 4;
	dst->dib_h.image_size = rowsize * dst->dib_h.height;
	dst->dib_h.x_pix_per_meter = 0;
	dst->dib_h.y_pix_per_meter = 0;
	dst->dib_h.colors_in_colortable = 0;
	dst->dib_h.important_color_count = 0;
	dst->bmp_h.file_size = dst->bmp_h.offset + dst->dib_h.image_size;

	rowsize_src1 = (src1->dib_h.bits_per_pix * src1->dib_h.width + 31) / 32 * 4;
	rowsize_src2 = (src2->dib_h.bits_per_pix * src2->dib_h.width + 31) / 32 * 4;
	row_length_src1 = src1->dib_h.width * (src1->dib_h.bits_per_pix / 8);
	row_length_src2 = src2->dib_h.width * (src1->dib_h.bits_per_pix / 8);

	ptrbmpdata = dst->pdata;
	for (i = 0; i < dst->dib_h.height; i++) {
		memcpy(ptrbmpdata, 
			src1->pdata + i * rowsize_src1, 
			row_length_src1);
		memcpy(ptrbmpdata + row_length_src1, 
			src2->pdata + i * rowsize_src2, 
			row_length_src2);
		if (rowsize > row_length_src1 + row_length_src2)
			memset(ptrbmpdata + row_length_src1 + row_length_src2,
				0,
				rowsize - row_length_src1 - row_length_src2);

		ptrbmpdata += rowsize;
	}
	return dst;
}

/*
 * 位图垂直合并
 */
bmp_file_t *
bmp_v_combin(const bmp_file_t *src1, const bmp_file_t *src2, bmp_file_t *dst)
{
	uint32_t rowsize;
	uint32_t rowsize_src1;
	uint32_t rowsize_src2;
	uint32_t row_length_src1;
	uint32_t row_length_src2;
	uint8_t *ptrbmpdata;

	memset(&dst->bmp_h, 0, sizeof(struct bmp_file_header));
	memset(&dst->dib_h, 0, sizeof(struct dib_header));

	dst->bmp_h.magic[0] = 'B';
	dst->bmp_h.magic[1] = 'M';
	dst->bmp_h.offset = sizeof(bmp_file_header_t) + sizeof(dib_header_t);
	dst->dib_h.dib_header_size = sizeof(dib_header_t);
	dst->dib_h.width = src1->dib_h.width;
	dst->dib_h.height = src1->dib_h.height + src2->dib_h.height;
	dst->dib_h.planes = 1;
	dst->dib_h.bits_per_pix = src1->dib_h.bits_per_pix;
	dst->dib_h.compression = 0;

	rowsize = (dst->dib_h.bits_per_pix * dst->dib_h.width + 31) / 32 * 4;
	dst->dib_h.image_size = rowsize * dst->dib_h.height;
	dst->dib_h.x_pix_per_meter = 0;
	dst->dib_h.y_pix_per_meter = 0;
	dst->dib_h.colors_in_colortable = 0;
	dst->dib_h.important_color_count = 0;
	dst->bmp_h.file_size = dst->bmp_h.offset + dst->dib_h.image_size;

	rowsize_src1 = (src1->dib_h.bits_per_pix * src1->dib_h.width + 31) / 32 * 4;
	rowsize_src2 = (src2->dib_h.bits_per_pix * src2->dib_h.width + 31) / 32 * 4;
	row_length_src1 = src1->dib_h.width * (src1->dib_h.bits_per_pix / 8);
	row_length_src2 = src2->dib_h.width * (src1->dib_h.bits_per_pix / 8);

	ptrbmpdata = dst->pdata;

	/*
	 * 按从上到下的顺序合并
	 * 若需从下到上的顺序， 则：
	 * memcpy(ptrbmpdata, src1->pdata, src1->dib_h.image_size);
	 * memcpy(ptrbmpdata + src1->dib_h.image_size, 
	 *        src2->pdata, src2->dib_h.image_size);
	 */
	memcpy(ptrbmpdata, src2->pdata, src2->dib_h.image_size);
	memcpy(ptrbmpdata + src2->dib_h.image_size, src1->pdata, src1->dib_h.image_size);
	return dst;
}

bmp_file_t *
bmp_h_combin_2(bmp_file_t *dst, const bmp_file_t *add)
{
	bmp_file_t tmp;

	if (dst->pdata == NULL) {
		memcpy(dst, add, sizeof(bmp_file_t));
		dst->pdata = malloc(dst->dib_h.image_size);
		memcpy(dst->pdata, add->pdata, add->dib_h.image_size);
	} else {
		memcpy(&tmp, dst, sizeof(bmp_file_t));
		tmp.pdata = malloc(tmp.dib_h.image_size);
		memcpy(tmp.pdata, dst->pdata, tmp.dib_h.image_size);
		dst->pdata = realloc(dst->pdata, dst->dib_h.image_size + add->dib_h.image_size);
		bmp_h_combin(&tmp, add, dst);

		free(tmp.pdata);
	}
	return dst;
}

/* 
 * 水平合并支持垂直分辨率不同位图
 * 垂直分辨率较小的位图上边将补齐空白
 */
bmp_file_t *
bmp_h_combin_3(bmp_file_t *dst, const bmp_file_t *add, int color_anti_flag)
{
	uint32_t h_diff;
	uint32_t rowsize;
	bmp_file_t bmp_blank;
	bmp_file_t tmp_bmp;

	if (dst->pdata == NULL) {
		memcpy(dst, add, sizeof(bmp_file_t));
		dst->pdata = malloc(dst->dib_h.image_size);
		memcpy(dst->pdata, add->pdata, add->dib_h.image_size);
		return dst;
	} 
	/*
	 * else 
	 */
	if (dst->dib_h.height > add->dib_h.height) {
		h_diff = dst->dib_h.height - add->dib_h.height;
		rowsize = (add->dib_h.bits_per_pix * add->dib_h.width + 31) / 32 * 4;
		bmp_blank.pdata = malloc(rowsize * h_diff);
		create_blank_bmp(&bmp_blank, add->dib_h.width, h_diff, dst->dib_h.bits_per_pix, color_anti_flag);
		memcpy(&tmp_bmp, &bmp_blank, sizeof(tmp_bmp));
		tmp_bmp.pdata = malloc(tmp_bmp.dib_h.image_size);
		memcpy(tmp_bmp.pdata, bmp_blank.pdata, tmp_bmp.dib_h.image_size);
		bmp_v_combin_2(&tmp_bmp, add);
		bmp_h_combin_2(dst, &tmp_bmp);

		free(tmp_bmp.pdata);
		free(bmp_blank.pdata);
	} else if (dst->dib_h.height < add->dib_h.height) {
		h_diff = add->dib_h.height - dst->dib_h.height;
		rowsize = (add->dib_h.bits_per_pix * add->dib_h.width + 31) / 32 * 4;
		bmp_blank.pdata = malloc(rowsize * h_diff);
		create_blank_bmp(&bmp_blank, dst->dib_h.width, h_diff, dst->dib_h.bits_per_pix, color_anti_flag);
		bmp_v_combin_2(&bmp_blank, dst);
		bmp_h_combin_2(&bmp_blank, add);
		memcpy(&dst->bmp_h, &bmp_blank.bmp_h, sizeof(bmp_file_header_t));
		memcpy(&dst->dib_h, &bmp_blank.dib_h, sizeof(dib_header_t));
		dst->pdata = realloc(dst->pdata, dst->dib_h.image_size);
		memcpy(dst->pdata, bmp_blank.pdata, dst->dib_h.image_size);

		free(bmp_blank.pdata);
	} else
		bmp_h_combin_2(dst, add);

	return dst;
}

/*
 * 合并的位图水平分辨率相同才能调用该函数
 */
bmp_file_t *
bmp_v_combin_2(bmp_file_t *dst, const bmp_file_t *add)
{
	bmp_file_t tmp;

	if (dst->pdata == NULL) {
		memcpy(dst, add, sizeof(bmp_file_t));
		dst->pdata = malloc(dst->dib_h.image_size);
		memcpy(dst->pdata, add->pdata, add->dib_h.image_size);
	} else {
		memcpy(&tmp, dst, sizeof(bmp_file_t));
		tmp.pdata = malloc(tmp.dib_h.image_size);
		memcpy(tmp.pdata, dst->pdata, tmp.dib_h.image_size);
		dst->pdata = realloc(dst->pdata, dst->dib_h.image_size + add->dib_h.image_size);
		bmp_v_combin(&tmp, add, dst);

		free(tmp.pdata);
	}
	return dst;
}

/* 
 * 垂直合并支持水平分辨率不同位图
 * 水平分辨率较小的位图左边将补齐空白
 */
bmp_file_t *
bmp_v_combin_3(bmp_file_t *dst, const bmp_file_t *add, int color_anti_flag)
{
	bmp_file_t tmp_bmp;
	bmp_file_t blank_bmp;
	uint32_t w_diff;
	uint32_t rowsize;

	if (dst->pdata == NULL) {
		memcpy(dst, add, sizeof(bmp_file_t));
		dst->pdata = malloc(dst->dib_h.image_size);
		memcpy(dst->pdata, add->pdata, add->dib_h.image_size);
		return dst;
	} 
	/*
	 * else 
	 */
	if (dst->dib_h.width > add->dib_h.width) {
		w_diff = dst->dib_h.width - add->dib_h.width;

		rowsize = (add->dib_h.bits_per_pix * w_diff + 31) / 32 * 4;
		blank_bmp.pdata = malloc(rowsize * add->dib_h.height);
		create_blank_bmp(&blank_bmp, w_diff, add->dib_h.height, add->dib_h.bits_per_pix, color_anti_flag);
		memcpy(&tmp_bmp, add, sizeof(tmp_bmp));
		tmp_bmp.pdata = malloc(tmp_bmp.dib_h.image_size);
		memcpy(tmp_bmp.pdata, add->pdata, tmp_bmp.dib_h.image_size);
		bmp_h_combin_2(&tmp_bmp, &blank_bmp);
		bmp_v_combin_2(dst, &tmp_bmp);

		free(tmp_bmp.pdata);
		free(blank_bmp.pdata);
	} else if (dst->dib_h.width < add->dib_h.width) {
		w_diff = add->dib_h.width - dst->dib_h.width;
		rowsize = (add->dib_h.bits_per_pix * w_diff + 31) / 32 * 4;
		blank_bmp.pdata = malloc(rowsize * dst->dib_h.height);
		create_blank_bmp(&blank_bmp, w_diff, dst->dib_h.height, add->dib_h.bits_per_pix, color_anti_flag);
		bmp_h_combin_2(dst, &blank_bmp);
		bmp_v_combin_2(dst, add);

		free(blank_bmp.pdata);
	} else
		bmp_v_combin_2(dst, add);

	return dst;
}
