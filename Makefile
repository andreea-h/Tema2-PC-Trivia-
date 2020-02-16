build:
	gcc -Wall trivia.c -o trivia -lncurses
run:
	valgrind --tool=memcheck --leak-check=full ./trivia intrebari1.txt intrebari2.txt intrebari3.txt
clean:
	rm -f trivia
