build:
	gcc -Wall -c src/linklayer.c -o protocol/linklayer.o 
clean:
	rm ../protocol/protocol.o
app:
	cd application && ./build.sh
cable:
	cd cable && make run
clean:
	cd src && make clean
	cd cable && make clean