all:
	gcc -o trab2SO main.c -lpthread -lrt -std=c99

clean:
	rm *.o trab2SO
