# kapish make file
all: kapish

kapish: kapish.o
	gcc -Wall -Werror -o kapish kapish.o

kapish.o: kapish.c
	gcc -c kapish.c

clean:
	rm kapish.o kapish
