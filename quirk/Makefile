CC = gcc
CFLAGS = -O3 -Wall
RM = rm -f

.PHONY: all
all: echo.o env.o help.o ls.o quirk.o

echo.o: echo.c echo.h
	$(CC) ${CFLAGS} -c echo.c

env.o: env.c env.h
	$(CC) ${CFLAGS} -c env.c

help.o: help.c help.h
	$(CC) ${CFLAGS} -c help.c

ls.h: ../shell.h
ls.o: ls.c ls.h
	$(CC) ${CFLAGS} -c ls.c

quirk.h: ../shell.h echo.h env.h help.h ls.h
quirk.o: quirk.c quirk.h
	$(CC) ${CFLAGS} -c quirk.c

.PHONY: clean
clean:
	$(RM) *.o
