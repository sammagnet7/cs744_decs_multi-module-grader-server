source: client-server

client-server: client server

client: gradlingclient.o
	@g++ -o submit -w gradlingclient.o

server: gradingserver.o
	@g++ -o server -w gradingserver.o

client.o: gradlingclient.cpp
	@g++ -c -w gradlingclient.cpp

server.o: gradingserver.cpp
	@g++ -c -w gradingserver.cpp


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
	@find . -type f -name "*.o" -exec rm {} \;