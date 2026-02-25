#!/bin/bash

# Define the ports to check
ports=(8080 9090)

for port in "${ports[@]}"; do
    # Find the process ID (PID) using the specified port
    pid=$(lsof -t -i :$port)

    if [ -n "$pid" ]; then
        echo "Process running on port $port with PID $pid. Killing..."
        kill -9 "$pid"
    else
        echo "No process found on port $port."
    fi
done

## script to run all 3 servers
make clean
make

./submission_server 8080 3 &
./grader_server	3 &
./query_server 9090 2