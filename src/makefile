initial:
	mkdir exec
compile :
	gcc -Wall writenoncanonical.c -o ../protocol/write.o
	gcc -Wall noncanonical.c -o ../protocol/read.o
write:
	./protocol/write.o /dev/tty$(port)
read:
	./protocol/read.o /dev/tty$(port)
clean:
	rm ../protocol/write.o ../protocol/read.o