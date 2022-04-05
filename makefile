cable:
	cd test	&& make run
build:
	cd scr && make clean && make compile && make $(mode) $(port)