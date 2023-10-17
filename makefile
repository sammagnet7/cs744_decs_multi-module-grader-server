source: client-server

obj= resources/sock.o resources/fileio.o
header= headers/sock.hpp headers/fileio.hpp

client-server: client server $(obj)


client: submit.o $(obj)
	g++ -o submit submit.o $(obj)

server: server.o $(obj)
	g++ -o server server.o $(obj)

client.o: submit.cpp $(header)
	g++ -c -w submit.cpp

server.o: server.cpp $(header)
	g++ -c -w server.cpp

sock.o: sock.cpp
	g++ -c -w resources/sock.cpp

fileio.o: fileio.cpp
	g++ -c -w resources/fileio.cpp


client-run:
	./submit localhost:8080 resources/test/source_P.cpp
	./submit localhost:8080 resources/test/source_OE.cpp
	./submit localhost:8080 resources/test/source_RE.cpp
	./submit localhost:8080 resources/test/source_CE.cpp

server-run: 
	./server 8080


.PHONY: clean

clean:
	@-rm submit server
	find . -type f -name "*.o" -exec rm {} \;