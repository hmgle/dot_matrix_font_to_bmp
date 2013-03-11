DEBUG = 1
.PHONY: all clean

CFLAGS = -Wall -O2

CROSS:=
CC = $(CROSS)gcc

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG=1
else
	CFLAGS += -DDEBUG=0
endif

TARGET = test utf8togb2312 gb2312tobmps bmps2bmp bmpsall2bmp bmpsallv2bmp \
	 text2bmp
TMPTARGET = test_bmp_h_combin create_blank_bmp_test combin_v_3_test

all: $(TARGET) $(TMPTARGET)

test: test.o dot_matrix_font_to_bmp.o

utf8togb2312: utf8togb2312.o encoding_convert.o dot_matrix_font_to_bmp.o

gb2312tobmps: gb2312tobmps.o dot_matrix_font_to_bmp.o

test_bmp_h_combin: test_bmp_h_combin.o dot_matrix_font_to_bmp.o

create_blank_bmp_test: create_blank_bmp_test.o dot_matrix_font_to_bmp.o bmp_io.o

bmps2bmp: bmps2bmp.o dot_matrix_font_to_bmp.o bmp_io.o

bmpsall2bmp: bmpsall2bmp.o dot_matrix_font_to_bmp.o bmp_io.o

bmpsallv2bmp: bmpsallv2bmp.o dot_matrix_font_to_bmp.o bmp_io.o

text2bmp: text2bmp.o dot_matrix_font_to_bmp.o

combin_v_3_test: combin_v_3_test.o dot_matrix_font_to_bmp.o bmp_io.o

clean:
	-rm -f *.o $(TARGET)
