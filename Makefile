all: compile exe

compile: 
	gcc -o shell Source.c

exe:
	./shell