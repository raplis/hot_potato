all: ringmaster player


ringmaster: ringmaster.cpp potato.h connect.h
	g++ -o ringmaster  -Wall -Werror ringmaster.cpp

player: player.cpp potato.h connect.h
	g++ -o player -Wall -Werror player.cpp
.PHONY:
	clean
clean:
	rm -rf *.o ringmaster player