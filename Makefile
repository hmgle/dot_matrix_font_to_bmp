DEBUG =
.PHONY: all clean

CFLAGS = -Wall

CROSS:=
CC = $(CROSS)gcc
STRIP = $(CROSS)strip

ifeq ($(DEBUG), 1)
	CFLAGS += -Wextra -DDEBUG=1 -g -O0
else
	CFLAGS += -DDEBUG=0 -O2
endif

SRCDIR = src
SRC := $(wildcard $(SRCDIR)/*.c)

ODIR := obj
OBJ  := $(patsubst %.c,$(ODIR)/%.o,$(SRC))

TARGET = utf8togb2312 gb2312tobmps bmps2bmp bmpsall2bmp bmpsallv2bmp \
	 text2bmp
TMPTARGET = test_bmp_h_combin create_blank_bmp_test combin_v_3_test test \
	combin_h_3_test test_encoding_detect

all: $(TARGET) $(TMPTARGET)

strip_target: all
	$(STRIP) $(TARGET)

test_encoding_detect: $(ODIR)/test_encoding_detect.o $(ODIR)/encoding_detect.o $(ODIR)/encoding_convert.o

test: $(ODIR)/test.o $(ODIR)/dot_matrix_font_to_bmp.o

utf8togb2312: $(ODIR)/utf8togb2312.o $(ODIR)/encoding_convert.o

gb2312tobmps: $(ODIR)/gb2312tobmps.o $(ODIR)/dot_matrix_font_to_bmp.o

test_bmp_h_combin: $(ODIR)/test_bmp_h_combin.o $(ODIR)/dot_matrix_font_to_bmp.o

create_blank_bmp_test: $(ODIR)/create_blank_bmp_test.o $(ODIR)/dot_matrix_font_to_bmp.o bmp_io.o

bmps2bmp: $(ODIR)/bmps2bmp.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

bmpsall2bmp: $(ODIR)/bmpsall2bmp.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

bmpsallv2bmp: $(ODIR)/bmpsallv2bmp.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

text2bmp: $(ODIR)/text2bmp.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/encoding_convert.o $(ODIR)/bmp_io.o $(ODIR)/encoding_detect.o

combin_v_3_test: $(ODIR)/combin_v_3_test.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

combin_h_3_test: $(ODIR)/combin_h_3_test.o $(ODIR)/dot_matrix_font_to_bmp.o $(ODIR)/bmp_io.o

$(OBJ): Makefile | $(ODIR)

$(ODIR):
	@mkdir $@

$(ODIR)/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f *.o $(TARGET) obj/*

sinclude $(SRC:.c=.d)

%d: %c
	@set -e; rm -f $@; \
		$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\(.*\)\.o[:]*,$(ODIR)/\1.o $@:,' < $@.$$$$ > $@; \
		rm -f $@.$$$$

vpath %.c src
vpath %.h src
vpath %.o obj
