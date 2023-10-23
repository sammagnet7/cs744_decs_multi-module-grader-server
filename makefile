source: client-server

client-server: client server

client: client.o fileio.o fileio.hpp
	g++ -std=c++17 -w -o submit gradingclient.o fileio.o

server: server.o fileio.o gradingserver_worker.o thread_pool.o fileio.hpp gradingserver_worker.hpp thread_pool.hpp
	g++ -std=c++17 -w -o server gradingserver.o fileio.o gradingserver_worker.o thread_pool.o

client.o: gradingclient.cpp
	g++ -std=c++17 -w -c gradingclient.cpp

server.o: gradingserver.cpp
	g++ -std=c++17 -w -c gradingserver.cpp

fileio.o: fileio.cpp fileio.hpp
	g++ -std=c++17 -w -c fileio.cpp

gradingserver_worker.o: gradingserver_worker.cpp
	g++ -std=c++17 -w -c gradingserver_worker.cpp

thread_pool.o: thread_pool.cpp
	g++ -std=c++17 -w -c thread_pool.cpp



client-run:
	./submit localhost:8080 test/source_P.cpp
	./submit localhost:8080 test/source_OE.cpp
	./submit localhost:8080 test/source_RE.cpp
	./submit localhost:8080 test/source_CE.cpp

server-run: 
	./server 8080


.PHONY: clean

clean:
	-rm submit server
	find . -type f -name "*.o" -exec rm {} \; -o -name "temp_files/*" -exec rm {} \;
	rm temp_files/*