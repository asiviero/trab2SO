all:
	gcc -o trab2SO *.c -lpthread -lrt -std=c99

clean:
	rm *.o trab2SO
