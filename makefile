CC=gcc
CFLAGS=-Wall -g

assemblermake:
	$(CC) -o shell shell.c $(CFLAGS)
