source: client

client: client.o fileio.o fileio.hpp
	g++ -std=c++17 -w -o submit gradingclient.o fileio.o

client.o: gradingclient.cpp
	g++ -std=c++17 -w -c gradingclient.cpp


fileio.o: fileio.cpp fileio.hpp
	g++ -std=c++17 -w -c fileio.cpp



.PHONY: clean

clean:
	-rm submit
	find . -type f -name "*.o" -exec rm {} \;