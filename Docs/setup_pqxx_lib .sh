#!/bin/bash

cd ..
sudo apt-get install libpq-dev

mkdir -p libraries
cd libraries/
git clone https://github.com/jtv/libpqxx.git
cd libpqxx
./configure --disable-shared
make
sudo make install
cd ..


###### Some more points to note ##########

#g++ -std=c++17 -w -o server .... -Wl,-rpath,lib/pg/lib/linux/pq -lpqxx -Llib/pg/lib/linux/pqxx -lpq -Llib/pg/lib/linux/pq
#postgres_util.o: postgres_util.cpp postgres_util.hpp
#	@g++ -std=c++17 -w -c postgres_util.cpp -Ilib/pg/include/

#thread_pool.o: thread_pool.cpp thread_pool.hpp
#	@g++ -std=c++17 -w -c thread_pool.cpp -Ilib/pg/include/

#OR
#g++ -std=c++17 -w -o server .... -lpqxx -lpq

#Also in the redis_util.hpp add header depending upon path: #include "lib/pg/include/pqxx/pqxx"
#OR
#Also in the postgres_util.hpp add header depending upon path: #include <pqxx/pqxx>
