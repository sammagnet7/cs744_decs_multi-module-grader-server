source: server
server: submission_server grader_server query_server

submission_server: submission_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o fileio.hpp gradingserver_worker.hpp thread_pool.hpp redis_util.hpp postgres_util.hpp
	#for LINUX
	@g++ -std=c++17 -w -o submission_server submission_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o lib/redis/lib/linux/libredis++.a lib/redis/lib/linux/libhiredis.a -pthread -Wl,-rpath,lib/pg/lib/linux/pq -lpqxx -Llib/pg/lib/linux/pqxx -lpq -Llib/pg/lib/linux/pq
	#for MAC
	#g++ -std=c++17 -w -o submission_server submission_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o lib/redis/lib/mac/libredis++.a lib/redis/lib/mac/libhiredis.a -pthread  -Wl,-rpath,lib/pg/lib/mac/pq -lpqxx -Llib/pg/lib/mac/pqxx -lpq -Llib/pg/lib/mac/pq

grader_server: grader_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o fileio.hpp gradingserver_worker.hpp thread_pool.hpp redis_util.hpp postgres_util.hpp
	#for LINUX
	@g++ -std=c++17 -w -o grader_server grader_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o lib/redis/lib/linux/libredis++.a lib/redis/lib/linux/libhiredis.a -pthread -Wl,-rpath,lib/pg/lib/linux/pq -lpqxx -Llib/pg/lib/linux/pqxx -lpq -Llib/pg/lib/linux/pq
	#for MAC
	#g++ -std=c++17 -w -o grader_server grader_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o lib/redis/lib/mac/libredis++.a lib/redis/lib/mac/libhiredis.a -pthread  -Wl,-rpath,lib/pg/lib/mac/pq -lpqxx -Llib/pg/lib/mac/pqxx -lpq -Llib/pg/lib/mac/pq

query_server: query_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o fileio.hpp gradingserver_worker.hpp thread_pool.hpp redis_util.hpp postgres_util.hpp
	#for LINUX
	@g++ -std=c++17 -w -o query_server query_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o lib/redis/lib/linux/libredis++.a lib/redis/lib/linux/libhiredis.a -pthread -Wl,-rpath,lib/pg/lib/linux/pq -lpqxx -Llib/pg/lib/linux/pqxx -lpq -Llib/pg/lib/linux/pq
	#for MAC
	#g++ -std=c++17 -w -o query_server query_server.o fileio.o gradingserver_worker.o thread_pool.o redis_util.o postgres_util.o lib/redis/lib/mac/libredis++.a lib/redis/lib/mac/libhiredis.a -pthread  -Wl,-rpath,lib/pg/lib/mac/pq -lpqxx -Llib/pg/lib/mac/pqxx -lpq -Llib/pg/lib/mac/pq


submission_server.o: submission_server.cpp
	@g++ -std=c++17 -w -c submission_server.cpp

grader_server.o: grader_server.cpp
	@g++ -std=c++17 -w -c grader_server.cpp

query_server.o: query_server.cpp
	@g++ -std=c++17 -w -c query_server.cpp

fileio.o: fileio.cpp fileio.hpp
	@g++ -std=c++17 -w -c fileio.cpp

gradingserver_worker.o: gradingserver_worker.cpp gradingserver_worker.hpp
	@g++ -std=c++17 -w -c gradingserver_worker.cpp -Ilib/pg/include/

redis_util.o: redis_util.cpp redis_util.hpp
	@g++ -std=c++17 -w -c redis_util.cpp

postgres_util.o: postgres_util.cpp postgres_util.hpp
	@g++ -std=c++17 -w -c postgres_util.cpp -Ilib/pg/include/

thread_pool.o: thread_pool.cpp thread_pool.hpp
	@g++ -std=c++17 -w -c thread_pool.cpp -Ilib/pg/include/


.PHONY: clean

clean:
	@-rm server
	@find . -type f -name "*.o" -exec rm {} \;
	@find . -type f -name "*.log" -exec rm {} \;
	@find . -type f -name "received*" -exec rm {} \;
	@rm -f submission_server grader_server query_server
	@rm -rf temp_files/