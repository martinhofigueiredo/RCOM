build src/linklayer.o : 
	gcc -Wall src/main.c src/linklayer.c src/util.c -o main
cable:
	gcc -Wall -o cable/cable.o cable/cable.c
runtx: main 
	./main /dev/ttyS0 tx src/penguin.gif
runrx: main
	./main /dev/ttyS0 rx penguinrx.gif
clean:
	rm src/linklayer.o && rm main && rm cable/cable.o