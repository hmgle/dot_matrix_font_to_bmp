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
	debug_print("pbmp_f->dib_h.image_size = %d", pbmp_f->dib_h.image_size);
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
	 uint16_t bits_per_pix)
{
	int i;
	int char_num;
	int char_bit;
	char bit;
	uint8_t *ptmp;

	ptmp = pdest;
	for (i = 0; i < width; i++) {
		char_num = i / 8;
		char_bit = 7 - i % 8;
		bit = ptrfontdata[char_num] & (1 << char_bit);
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
	     uint16_t bits_per_pix)
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
			 bits_per_pix);
		ptrbmpdata += rowsize;
	}
}

uint32_t 
gb2312code_to_fontoffset(uint32_t gb2312code)
{
	uint32_t fontoffset;

	fontoffset = (gb2312code % 0x100 - 0xA1) * 94
		     + (gb2312code / 0x100 - 0xA1);
	fontoffset *= 32;
	return fontoffset;
}

/*
 * 位图水平合并
 */
bmp_file_t *
bmp_h_combin(const bmp_file_t *src1, const bmp_file_t *src2, bmp_file_t *dst)
{
	int i;
	uint32_t rowsize;
	uint32_t rowsize_src1;
	uint32_t rowsize_src2;
	uint32_t row_length_src1;
	uint32_t row_length_src2;
	uint8_t *ptrbmpdata;

	debug_print("dst->pdata is %#x", dst->pdata);
	memset(&dst->bmp_h, 0, sizeof(struct bmp_file_header));
	memset(&dst->dib_h, 0, sizeof(struct dib_header));

	debug_print("dst->pdata is %#x", dst->pdata);
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
	debug_print("dst->bmp_h.file_size = %d", dst->bmp_h.file_size);

	rowsize_src1 = (src1->dib_h.bits_per_pix * src1->dib_h.width + 31) / 32 * 4;
	rowsize_src2 = (src2->dib_h.bits_per_pix * src2->dib_h.width + 31) / 32 * 4;
	row_length_src1 = src1->dib_h.width * (src1->dib_h.bits_per_pix / 8);
	row_length_src2 = src2->dib_h.width * (src1->dib_h.bits_per_pix / 8);

	ptrbmpdata = dst->pdata;
	for (i = 0; i < dst->dib_h.height; i++) {
		debug_print("row_length_src1 = %d", row_length_src1);
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
