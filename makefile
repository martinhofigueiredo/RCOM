cable:
	cd test	&& make run
run:
	cd scr && make clean && make compile && make $(mode) $(port)
build:
	cd scr && make clean && make compile