build main: 
	gcc -Wall src/main.c src/linklayer.c src/util.c -o main
cablefile:
	gcc -Wall cable/cable.c -o cable/cable.o && ./cable/cable.o
tx: main 
	./main /dev/ttyS0 tx penguin.gif
rx: main
	./main /dev/ttyS0 rx penguinrx.gif
tunnel:
	sudo socat -d -d PTY,link=/dev/ttyS10,mode=777 PTY,link=/dev/ttyS11,mode=777	
virtualtx: main 
	./main /dev/ttyS10 tx penguin.gif
virtualrx: main
	./main /dev/ttyS11 rx penguinrx.gif
clean:
	rm main && rm cable/cable.o && rm penguinrx.gif