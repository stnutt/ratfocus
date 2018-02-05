PREFIX ?= /usr
CC ?= gcc
CFLAGS += -O3 -pedantic -Wall -lX11
BIN = ratfocus
SRC = ratfocus.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean install

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BIN) $(OBJ)

install: $(BIN)
	install -m 755 $(BIN) $(PREFIX)/bin
