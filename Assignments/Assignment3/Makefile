all: fish pellet swim_mill
fish : fish.o
	gcc -o fish fish.o
fish.o : fish.c
	gcc -c fish.c -o fish.o
pellet : pellet.o
	gcc -o pellet pellet.o
pellet.o : pellet.c
	gcc -c pellet.c -o pellet.o
swim_mill : swim_mill.o
	gcc -o swim_mill swim_mill.c
swim_mill.o : swim_mill.c
	gcc -c swim_mill.c -o swim_mill.o
