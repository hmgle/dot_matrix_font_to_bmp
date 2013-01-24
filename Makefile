.PHONY: all clean

CFLAGS = -Wall -O2

TARGET = test

all: $(TARGET)

test: test.o dot_matrix_font_to_bmp.o

clean:
	-rm -f *.o $(TARGET)
