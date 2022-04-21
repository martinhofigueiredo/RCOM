build main: 
	gcc -Wall src/main.c src/linklayer.c src/util.c -o main
cable:
	gcc -Wall cable/cable.c -o cable/cable.o
runtx: main 
	./main /dev/ttyS0 tx src/penguin.gif
runrx: main
	./main /dev/ttyS0 rx penguinrx.gif
clean:
	rm src/linklayer.o && rm main && rm cable/cable.o