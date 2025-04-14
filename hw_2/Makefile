CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared

LIB_NAME = libapple.so
TARGET = app

OBJS = apple.o

all: $(LIB_NAME) $(TARGET)

$(LIB_NAME): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET): main.o $(LIB_NAME)
	$(CC) -o $@ main.o -L. -lapple

main.o: main.c apple.h
	$(CC) $(CFLAGS) -c main.c

apple_price.o: apple.c apple.h
	$(CC) $(CFLAGS) -c apple.c

clean:
	rm -f *.o *.so $(TARGET)
