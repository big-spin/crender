CC = gcc

TARGET = crender

CFLAGS = -Wall -Wextra -pedantic -O3
LIBS = -lm -lX11 -lXfixes

DEBUGFLAGS = -g -fsanitize=address -fno-omit-frame-pointer

SRC = src/main.c

.PHONY: build debug clean install uninstall

build: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

debug: $(SRC)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/bin/$(TARGET)

uninstall:
	sudo rm -f /usr/bin/$(TARGET)
