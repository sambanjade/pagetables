CC = gcc
CFLAGS = -Wall -g -std=c11
LDFLAGS =

AR = ar
ARFLAGS = rcs

TARGET = libmlpt.a
OBJECTS = mlpt.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

mlpt.o: mlpt.c mlpt.h config.h
	$(CC) $(CFLAGS) -c mlpt.c -o mlpt.o

clean:
	rm -f $(OBJECTS) $(TARGET)
