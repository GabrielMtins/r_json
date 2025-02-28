all:
	gcc -g src/*.c -I./include -std='c89' -pedantic -Wall -Wextra -o main
