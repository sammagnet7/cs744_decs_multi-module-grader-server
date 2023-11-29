#!/bin/bash

### This script is used to send the server side generated logs to the client side when finally server is killed.

#path to client machine where to send the logs
client_machine_path="ankur@192.168.0.105:~/Documents"
 

averageCpuUtilization=0
averageActiveThreads=0
count=0

mkdir -p temp_files

log_file="temp_files/server_snapshot.log"
nc_output_file="temp_files/server_snapshot_nc.log"
avg_q_len_file="temp_files/avgQ.log"
serviceTime_file="temp_files/serviceTime.log"


while [ true ]; do
    COND=$(ps aux | grep "grader_server" | wc -l)

    if [ $COND -gt 1 ]; then
        # Get the current timestamp
        timestamp=$(date "+%H:%M:%S")

        # Start listening on port 12345 and append to the nc output file with timestamp
        { echo "$timestamp"; nc -l -p 12345; } >> "$nc_output_file" &

        # Get active threads and CPU utilization with a timestamp and append to the same line in the log file
        threads_by_submission_server=$(ps -eLf | grep "./submission_server 8080" | awk 'NR==1{print $6}')
        threads_by_query_server=$(ps -eLf | grep "./query_server 9090" | awk 'NR==1{print $6}')
        threads_by_grader_server=$(ps -eLf | grep "./grader_server" | awk 'NR==1{print $6}')

        threads=$(($threads_by_submission_server+$threads_by_query_server+$threads_by_grader_server))
        cpu_utilization=$(top -b -n 1 | awk -F',' '/%Cpu/ {print $1}' | sed 's/[^0-9.]//g')
        echo "threads :" $threads " cpu Utilization: " $cpu_utilization
        echo "$timestamp $threads $cpu_utilization" >> "$log_file"

        sleep 3
    else
        echo "Starting transferring the log files to the client machine....."
        # Copy .log to a remote location using scp
        scp "$log_file" "$nc_output_file" "$avg_q_len_file" "$serviceTime_file" $client_machine_path
        pkill -f "nc -l -p 12345"

        #At deployment time for deleting uncomment below line
        #rm -rf temp_files

        break
    fi
done