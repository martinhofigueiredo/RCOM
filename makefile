run:
	make build && make app && make cable &&cd src && make $(mode) port=$(port)
build:
	cd src && make compile
app:
	cd application && ./build.sh
cable:
	cd cable && make run
clean:
	cd src && make clean
	cd cable && make clean