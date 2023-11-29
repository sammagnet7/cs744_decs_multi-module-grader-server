#!/bin/bash

client_machine_path="ankur@10.96.20.80:~/Documents"

#variable 
averageCpuUtilization=0
averageActiveThreads=0
count=0

mkdir -p temp_files

log_file="temp_files/server_snapshot.log"
nc_output_file="temp_files/server_snapshot_nc.log"
avg_q_len_file="temp_files/avgQ.log"

while [ true ]; do
    COND=$(ps aux | grep "server 8080" | wc -l)
    if [ $COND -gt 1 ]; then
        # Get the current timestamp
        timestamp=$(date "+%H:%M:%S")

        # Start listening on port 12345 and append to the nc output file with timestamp
        { echo "$timestamp"; nc -l -p 12345; } >> "$nc_output_file" &
        # Get active threads and CPU utilization with a timestamp and append to the same line in the log file
        threads=$(ps -eLf | grep "./server 8080" | awk 'NR==1{print $6}')
        cpu_utilization=$(top -b -n 1 | awk -F',' '/%Cpu/ {print $1}' | sed 's/[^0-9.]//g')
        
        if [ "$cpu_utilization" != "0.0" ]; then
            echo "threads :" $threads " cpu Utilization: " $cpu_utilization
            echo "$timestamp $threads $cpu_utilization" >> "$log_file"
        fi

        sleep 3
    else
    	echo "Starting transferring the log files to the client machine....."
        # Copy .log to a remote location using scp
        scp "$log_file" "$nc_output_file" "$avg_q_len_file" "$serviceTime_file" $client_machine_path
        pkill -f "nc -l -p 12345"
        rm -rf temp_files
        break
    fi
done
