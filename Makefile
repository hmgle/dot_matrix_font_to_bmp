DEBUG =
.PHONY: all clean

CFLAGS = -Wall -O2

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG=1
else
	CFLAGS += -DDEBUG=0
endif

TARGET = test utf8togb2312 gb2312tobmp

all: $(TARGET)

test: test.o dot_matrix_font_to_bmp.o

utf8togb2312: utf8togb2312.o encoding_convert.o dot_matrix_font_to_bmp.o

gb2312tobmp: gb2312tobmp.o dot_matrix_font_to_bmp.o

clean:
	-rm -f *.o $(TARGET)
