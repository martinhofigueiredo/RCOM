cable:
	cd cable	&& make run
run:
	cd scr && make compile && make $(mode) port=$(port)
build:
	cd scr && make compile
clean:
	cd scr && make clean
	cd cable && make clean