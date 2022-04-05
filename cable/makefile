compile:
	gcc -Wall -o cable cable.c
run:
	make compile
	sudo socat -d -d PTY,link=/tmp/ttyS10,mode=777 PTY,link=/tmp/ttyS11,mode=777 &
	sudo ./cable 
clean:
	rm cable