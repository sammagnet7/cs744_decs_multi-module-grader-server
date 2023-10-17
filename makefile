source: client-server

client-server: client server

client: client.o
	@g++ -o submit client.o

server: server.o
	@g++ -o server server.o

client.o: gradingclient.cpp
	@g++ -o client.o -w -c gradingclient.cpp

server.o: gradingserver.cpp
	@g++ -std=c++17 -o server.o -w -c gradingserver.cpp


client-run:
	./submit localhost:8080 test/source_P.cpp
	./submit localhost:8080 test/source_OE.cpp
	./submit localhost:8080 test/source_RE.cpp
	./submit localhost:8080 test/source_CE.cpp

server-run: 
	./server 8080


.PHONY: clean

clean:
	@-rm submit server
	@find . -type f -name "*.o" -exec rm {} \; -o -name "output_*" -exec rm {} \;