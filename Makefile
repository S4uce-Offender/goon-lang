SRC = $(wildcard src/*.c)

build:
	gcc -g $(SRC) -o gooner
run:
	./gooner scripts/test.goon

leak-check:
	valgrind --leak-check=full ./gooner scripts/test.goon