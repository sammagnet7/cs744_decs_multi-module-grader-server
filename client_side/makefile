source: submit_async submit

submit_async: client_async.o fileio.o fileio.hpp
	@g++ -std=c++17 -w -o submit_async gradingclient_async.o fileio.o

submit: client.o fileio.o fileio.hpp
	@g++ -std=c++17 -w -o submit gradingclient.o fileio.o

client_async.o: gradingclient_async.cpp
	@g++ -std=c++17 -w -c gradingclient_async.cpp

client.o: gradingclient.cpp
	@g++ -std=c++17 -w -c gradingclient.cpp

fileio.o: fileio.cpp fileio.hpp
	@g++ -std=c++17 -w -c fileio.cpp



.PHONY: clean

clean:
	@rm submit_async
	@rm submit
	@find . -type f -name "*.o" -exec rm {} \;