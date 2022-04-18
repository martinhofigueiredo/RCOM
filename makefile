build protocol/linklayer.o : 
	gcc -Wall -c src/linklayer.c -o protocol/linklayer.o 
app:
	cd application \
	gcc -Wall main.c ../protocol/*.o -o mainu
cable:
	cd cable \
	gcc -Wall -o cable cable.c \
	sudo socat -d -d PTY,link=/dev/ttyS10,mode=777 PTY,link=/dev/ttyS11,mode=777 &
	sudo ./cable/cable 
run: protocol/linklayer.o cable/cable application/main
	./application/main /dev/ttS10 tx application/penguin-received.gif
	./application/main /dev/ttS11 rx application/penguin-received.gif
clean:
	rm protocol/linklayer.o \
	rm application/main \
	rm cable/cable \