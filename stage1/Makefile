all: parser

parser: lex.yy.o parse.tab.o tree.o codegen.o
	gcc -o parser lex.yy.o parse.tab.o tree.o codegen.o

codegen.o: codegen.c
	gcc -c codegen.c

lex.yy.c: lex.l parse.tab.h
	flex lex.l

parse.tab.c parse.tab.h: parse.y
	bison -d parse.y

lex.yy.o: lex.yy.c
	gcc -c lex.yy.c

parse.tab.o: parse.tab.c
	gcc -c parse.tab.c

tree.o: tree.c
	gcc -c tree.c

clean:
	rm lex.yy.o parse.tab.o tree.o lex.yy.c parse.tab.c parse.tab.h
