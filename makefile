build protocol/linklayer.o : 
	gcc -Wall -c src/linklayer.c -o src/linklayer.o 
app:
	gcc -Wall src/main.c src/linklayer.o -o main
cable:
	gcc -Wall -o cable/cable cable/cable.c && sudo socat -d -d PTY,link=/dev/ttyS10,mode=777 PTY,link=/dev/ttyS11,mode=777 & \
	sudo ./cable/cable 
run: src/linklayer.o cable/cable src/main
	./main /dev/ttS10 tx penguin-received.gif
	./main /dev/ttS11 rx penguin-received.gif
clean:
	rm src/linklayer.o && rm main && rm cable/cable