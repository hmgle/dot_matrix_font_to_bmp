DEBUG =
.PHONY: all clean

CFLAGS = -Wall -O2

CROSS:=
CC = $(CROSS)gcc
STRIP = $(CROSS)strip

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG=1
else
	CFLAGS += -DDEBUG=0
endif

SRCDIR = src
SRC := $(wildcard $(SRCDIR)/*.c)

ODIR := obj
OBJ  := $(patsubst %.c,$(ODIR)/%.o,$(SRC))

TARGET = utf8togb2312 gb2312tobmps bmps2bmp bmpsall2bmp bmpsallv2bmp \
	 text2bmp
TMPTARGET = test_bmp_h_combin create_blank_bmp_test combin_v_3_test test

all: $(TARGET)

strip_target: all
	$(STRIP) $(TARGET)

test: $(ODIR)/test.o $(ODIR)/dot_matrix_font_to_bmp.o

utf8togb2312: $(ODIR)/utf8togb2312.o $(ODIR)/encoding_convert.o

gb2312tobmps: $(ODIR)/gb2312tobmps.o $(ODIR)/dot_matrix_font_to_bmp.o

test_bmp_h_combin: $(ODIR)/test_bmp_h_combin.o $(ODIR)/dot_matrix_font_to_bmp.o

create_blank_bmp_test: $(ODIR)/create_blank_bmp_test.o $(ODIR)/dot_matrix_font_to_bmp.o bmp_io.o

bmps2bmp: $(ODIR)/bmps2bmp.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

bmpsall2bmp: $(ODIR)/bmpsall2bmp.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

bmpsallv2bmp: $(ODIR)/bmpsallv2bmp.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

text2bmp: $(ODIR)/text2bmp.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/encoding_convert.o $(ODIR)/bmp_io.o

combin_v_3_test: $(ODIR)/combin_v_3_test.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

$(OBJ): Makefile | $(ODIR)

$(ODIR):
	@mkdir $@

$(ODIR)/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f *.o $(TARGET) obj/*

vpath %.c src
vpath %.h src
vpath %.o obj
