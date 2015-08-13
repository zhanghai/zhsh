CC = gcc
CFLAGS = -O3
RM = rm -f

.PHONY: default_target
default_target: all

echo.o: echo.c echo.h
	$(CC) ${CFLAG} -c echo.c

env.o: env.c env.h
	$(CC) ${CFLAG} -c env.c

help.o: help.c help.h
	$(CC) ${CFLAG} -c help.c

ls.h: ../shell.h
ls.o: ls.c ls.h
	$(CC) ${CFLAG} -c ls.c

quirk.h: ../shell.h echo.h env.h help.h ls.h
quirk.o: quirk.c quirk.h
	$(CC) ${CFLAG} -c quirk.c

.PHONY: all
all: echo.o env.o help.o ls.o quirk.o

.PHONY: clean
clean:
	$(RM) *.o
