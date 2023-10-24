#!/bin/bash

#variable 
averageCpuUtilization=0
averageActiveThreads=0
count=0

while [ true ];
do 
    COND=$(ps aux|grep "server 8080"|wc -l)
    if [ $COND -gt 1 ]; then

        # nlwp=$(ps -eLf|grep "./server 8080"|awk 'NR==1{print $1}'| awk '{nlwp=$6} END{print nlwp}')
        nlwp=$(ps -eLf|grep "./server 8080"|awk 'NR==1{print $6}')
        echo "nlwp: " $nlwp
        averageActiveThreads=$((averageActiveThreads + nlwp))

        count=$((count + 1))

        top_output=$(top -b -n 1| awk -F',' '/%Cpu/ {print $1}'|sed 's/[^0-9.]//g')
        echo "cpu: " $top_output 
        averageCpuUtilization=$(echo "$averageCpuUtilization + $top_output" | bc)
        # echo "server is running"
        sleep 5
    else
        # echo $averageCpuUtilization 
        # echo $averageActiveThreads

        awk "BEGIN { print \"averageCpuUtilization: \" $averageCpuUtilization/$count }"
        awk "BEGIN { print \"averageActiveThreads: \" $averageActiveThreads/$count }"

        # echo "server stopped"
        break
    fi
done
