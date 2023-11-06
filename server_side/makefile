source: server


server: server.o fileio.o gradingserver_worker.o thread_pool.o fileio.hpp gradingserver_worker.hpp thread_pool.hpp
	g++ -std=c++17 -w -o server gradingserver.o fileio.o gradingserver_worker.o thread_pool.o

server.o: gradingserver.cpp
	g++ -std=c++17 -w -c gradingserver.cpp

fileio.o: fileio.cpp fileio.hpp
	g++ -std=c++17 -w -c fileio.cpp

gradingserver_worker.o: gradingserver_worker.cpp
	g++ -std=c++17 -w -c gradingserver_worker.cpp

thread_pool.o: thread_pool.cpp
	g++ -std=c++17 -w -c thread_pool.cpp



.PHONY: clean

clean:
	-rm server
	find . -type f -name "*.o" -exec rm {} \;
	find . -type f -name "*.log" -exec rm {} \;
	find . -type f -name "received*" -exec rm {} \;
	rm -rf temp_files/