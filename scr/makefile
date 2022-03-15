compile :
	gcc -Wall -o exec/writenoncanonical writenoncanonical.c
	gcc -Wall -o exec/noncanonical noncanonical.c
write:
	./exec/writenoncanonical /dev/tty$(port)
read:
	./exec/noncanonical /dev/tty$(port)