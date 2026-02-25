#!/bin/bash

cd ..
mkdir -p libraries
cd libraries/
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo make install

################## Output returned ################
#mkdir -p /usr/local/include/hiredis /usr/local/include/hiredis/adapters /usr/local/lib
#cp -pPR hiredis.h async.h read.h sds.h alloc.h sockcompat.h /usr/local/include/hiredis
#cp -pPR adapters/*.h /usr/local/include/hiredis/adapters
#cp -pPR libhiredis.so /usr/local/lib/libhiredis.so.1.2.1-dev
#cd /usr/local/lib && ln -sf libhiredis.so.1.2.1-dev libhiredis.so && ln -sf libhiredis.so.1.2.1-dev libhiredis.so.1
#cp -pPR libhiredis.a /usr/local/lib
#mkdir -p /usr/local/lib/pkgconfig
#cp -pPR hiredis.pc /usr/local/lib/pkgconfig
###################################################

cd ..
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus/
mkdir build

cd build
#If CMAKE isnot there
sudo apt  install cmake

cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=17 ..
make
sudo make install
cd ../../../server_side/
make clean
make

################## Output returned ################
#-- Install configuration: "Release"
#-- Installing: /usr/local/lib/libredis++.a
#-- Installing: /usr/local/lib/libredis++.so.1.3.10
#-- Installing: /usr/local/lib/libredis++.so.1
#-- Set runtime path of "/usr/local/lib/libredis++.so.1.3.10" to ""
#-- Installing: /usr/local/lib/libredis++.so
#-- Installing: /usr/local/share/cmake/redis++/redis++-targets.cmake
#-- Installing: /usr/local/share/cmake/redis++/redis++-targets-release.cmake
#-- Installing: /usr/local/include/sw/redis++/cmd_formatter.h
#-- Installing: /usr/local/include/sw/redis++/command.h
#-- Installing: /usr/local/include/sw/redis++/command_args.h
#-- Installing: /usr/local/include/sw/redis++/command_options.h
#-- Installing: /usr/local/include/sw/redis++/connection.h
#-- Installing: /usr/local/include/sw/redis++/connection_pool.h
#-- Installing: /usr/local/include/sw/redis++/cxx_utils.h
#-- Installing: /usr/local/include/sw/redis++/errors.h
#-- Installing: /usr/local/include/sw/redis++/hiredis_features.h
#-- Installing: /usr/local/include/sw/redis++/tls.h
#-- Installing: /usr/local/include/sw/redis++/pipeline.h
#-- Installing: /usr/local/include/sw/redis++/queued_redis.h
#-- Installing: /usr/local/include/sw/redis++/queued_redis.hpp
#-- Installing: /usr/local/include/sw/redis++/redis++.h
#-- Installing: /usr/local/include/sw/redis++/redis.h
#-- Installing: /usr/local/include/sw/redis++/redis.hpp
#-- Installing: /usr/local/include/sw/redis++/redis_cluster.h
#-- Installing: /usr/local/include/sw/redis++/redis_cluster.hpp
#-- Installing: /usr/local/include/sw/redis++/redis_uri.h
#-- Installing: /usr/local/include/sw/redis++/reply.h
#-- Installing: /usr/local/include/sw/redis++/sentinel.h
#-- Installing: /usr/local/include/sw/redis++/shards.h
#-- Installing: /usr/local/include/sw/redis++/shards_pool.h
#-- Installing: /usr/local/include/sw/redis++/subscriber.h
#-- Installing: /usr/local/include/sw/redis++/transaction.h
#-- Installing: /usr/local/include/sw/redis++/utils.h
#-- Installing: /usr/local/include/sw/redis++/patterns/redlock.h
#-- Installing: /usr/local/share/cmake/redis++/redis++-config.cmake
#-- Installing: /usr/local/share/cmake/redis++/redis++-config-version.cmake
#-- Installing: /usr/local/lib/pkgconfig/redis++.pc
###################################################

###### Some more points to note ##########
#We need 2 files for passing as arguments while building the executable and the include folder (Default: /usr/local/include) like below:
#hiredis/libhiredis.a
#redis-plus-plus/build/libredis++.a

#g++ -std=c++17 -w -o server .... lib/redis_linux/libredis++.a lib/redis_linux/libhiredis.a -pthread
#OR
#g++ -std=c++17 -w -o server .... /usr/local/lib/libhiredis.a /usr/local/lib/libredis++.a -pthread

#Also in the redis_util.hpp add header depending upon path: #include "lib/redis_linux/sw/redis++/redis++.h"
#OR
#Also in the redis_util.hpp add header depending upon path: #include <sw/redis++/redis++.h>
