#include <stdio.h>
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
	printf("%s %d pbmp_f->dib_h.image_size = %d\n", __func__, __LINE__, pbmp_f->dib_h.image_size);
	pbmp_f->dib_h.x_pix_per_meter = 0;
	pbmp_f->dib_h.y_pix_per_meter = 0;
	pbmp_f->dib_h.colors_in_colortable = 0;
	pbmp_f->dib_h.important_color_count = 0;
	pbmp_f->bmp_h.file_size = pbmp_f->bmp_h.offset + pbmp_f->dib_h.image_size;
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
#if 0
		char_num = i / 8;
		char_bit = i % 8;
		bit = ptrfontdata[char_num] & (1 << char_bit);
#else
		char_num = i / 8;
		char_bit = 7 - i % 8;
		bit = ptrfontdata[char_num] & (1 << char_bit);
#endif
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
	for (i = 0; i < height; i++) {
		conv_row(ptrfontdata + (width + 7) / 8 * i, 
			 width, 
			 ptrbmpdata,
			 bits_per_pix);
		ptrbmpdata += rowsize;
	}
}
