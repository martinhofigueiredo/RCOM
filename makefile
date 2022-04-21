build src/linklayer.o : 
	gcc -Wall -c src/linklayer.c -o src/linklayer.o 
app: src/linklayer.o
	gcc -Wall src/main.c src/linklayer.o -o main
cable:
	gcc -Wall -o cable/cable.o cable/cable.c
runtx: src/main 
	./main /dev/ttyS0 tx penguin-received.gif
runrx:
	./main /dev/ttyS0 rx penguin-received.gif
clean:
	rm src/linklayer.o && rm main && rm cable/cable.o