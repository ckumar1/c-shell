msyh: shell.o
gcc -o mysh shell.o -lm

shell.o: shell.c
gcc -O -Wall -c shell.c

clean:
rm -f shell.o mysh


