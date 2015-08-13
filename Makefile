FLEX = flex
LEMON = lemon -q
CC = gcc
CFLAGS = -O3 -Wall
CFLAGS_FLEX = -Wno-unused-function
CFLAGS_LEMON = -Wno-unused-variable
MAKE = make
RM = rm -f

.PHONY: all
all: zhsh

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

line_lexer.h line_lexer.c: line_lexer.l.intermediate ;
.INTERMEDIATE: line_lexer.l.intermediate
line_lexer.l.intermediate: line_lexer.l
	$(FLEX) --header-file=line_lexer.h --outfile=line_lexer.c line_lexer.l

line_lexer.o: line_lexer.c line_parser.h
	$(CC) $(CFLAGS) $(CFLAGS_FLEX) -c line_lexer.c

line_syntax.h: util.h
line_syntax.o: line_syntax.c line_syntax.h
	$(CC) $(CFLAGS) -c line_syntax.c

line_parser.h line_parser.c: line_parser.y.intermediate ;
.INTERMEDIATE: line_parser.y.intermediate
line_parser.y.intermediate: line_parser.y
	$(LEMON) line_parser.y

line_parser.o: line_parser.c line_syntax.h util.h
	$(CC) $(CFLAGS) $(CFLAGS_LEMON) -c line_parser.c

parser.h: line_syntax.h
parser.o: parser.c parser.h line_lexer.h
	$(CC) $(CFLAGS) -c parser.c

shell.h: util.h
shell.o: shell.c shell.h line_syntax.h line_parser.h parser.h quirk/quirk.h
	$(CC) $(CFLAGS) -c shell.c

.PHONY: quirk
quirk:
	$(MAKE) -C quirk

zhsh: util.o line_lexer.o line_syntax.o line_parser.o parser.o quirk shell.o
	$(CC) $(CFLAGS) -o zhsh util.o line_lexer.o line_syntax.o line_parser.o parser.o quirk/echo.o quirk/env.o quirk/help.o quirk/ls.o quirk/quirk.o shell.o -lreadline

.PHONY: clean
clean:
	$(MAKE) -C quirk clean
	$(RM) zhsh line_lexer.{h,c} line_parser.{h,c} *.o
