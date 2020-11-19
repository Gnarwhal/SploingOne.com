.PHONY: run
run: server
	./server

server: out/ out/TCPServerSocket.o src/main.cpp
	g++-10 -std=c++2a $(flags) src/main.cpp out/TCPServerSocket.o -o $@

out/TCPServerSocket.o: out/ src/net/TCPServerSocket.hpp src/net/TCPServerSocket.cpp
	g++-10 -std=c++2a -c $(flags) src/net/TCPServerSocket.cpp -o $@

out/:
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf out/*
	rm server
