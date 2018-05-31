all: spi-server

CC ?= gcc

spi-server: spi-server.c
	$(CC) -Wall -pthread spi-server.c -o spi-server

clean:
	rm -f spi-server
