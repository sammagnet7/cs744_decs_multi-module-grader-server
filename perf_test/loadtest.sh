#!/bin/bash

#Enter the server ip:port to submit requests
serverip_port=10.157.3.213:8080

#for bash debugging
#set -x 


if [ $# -ne 4 ]; then
	echo "Usage: $0 <numClients> <loopNum> <sleepTimeSeconds> <timeout-seconds>"
	exit
fi

numClients=$1
loopNum=$2
sleepTimeSeconds=$3
timeout=$4

echo "Arguments passed: #clients: "$numClients" #loops: "$loopNum" sleep: "$sleepTimeSeconds" timeout: "$timeout

mkdir  temp_files

counter=$numClients
for (( i = 0; i < $counter; i++ )); do
	#../client_side/submit 10.157.3.213:8080 ../client_side/test/source_P.cpp 10 0.5 0.9
	../client_side/submit $serverip_port ../client_side/test/source_P.cpp $loopNum $sleepTimeSeconds $timeout > temp_files/output_$i.txt 2>&1 &
done

wait


#echo "==================== done ====================="


###################################
#storing each file data into array:
###################################

declare -a avg_resp_times
declare -a success_counts
declare -a per_client_totalRequests
declare -a per_client_throughput
declare -a per_client_timeoutrequests
declare -a per_client_errorRequests
#declare -a acc_resp_times
#declare -a loop_times

for (( i = 0; i < $counter; i++ )); do


	avg_resp_times[$i]=$(cat temp_files/output_$i.txt | awk '/Average response time/ {print $0}' | awk -F: '{print $2;}')
	success_counts[$i]=$(cat temp_files/output_$i.txt | awk '/Number of successful responses/ {print $0}' | awk -F: '{print $2;}')

	per_client_totalRequests[$i]=$( cat temp_files/output_$i.txt | awk '/Individual client total requests/ {print $0}' | awk -F: '{print $2;}'  )
	per_client_throughput[$i]=$( cat temp_files/output_$i.txt | awk '/Individual client throughput/ {print $0}' | awk -F: '{print $2;}'  )
	per_client_timeoutrequests[$i]=$( cat temp_files/output_$i.txt | awk '/Individual client timeout requests/ {print $0}' | awk -F: '{print $2;}'  )
	per_client_errorRequests[$i]=$( cat temp_files/output_$i.txt | awk '/Individual client other error requests/ {print $0}' | awk -F: '{print $2;}'  )

	#acc_resp_times[$i]=$(cat temp_files/output_$i.txt | awk '/Accumulated response time/ {print $0}' | awk -F: '{print $2;}')
	#loop_times[$i]=$(cat temp_files/output_$i.txt | awk '/Time taken for completing client loop/ {print $0}' | awk -F: '{print $2;}')

done

#####################################
#calculating outputs for each clients:
#####################################

sum_of_success=0
sum_of_resp_time=0

sum_of_totRequests=0
sum_of_throughput=0
sum_of_timeoutrequests=0
sum_of_errRequets=0



for (( i = 0; i < $counter; i++ )); do

	sum_of_success=$( awk '{print $1+$2}' <<<"${sum_of_success} ${success_counts[$i]}" )
	sum_of_resp_time=$( awk '{print $1+($2*$3)}' <<<"${sum_of_resp_time} ${success_counts[$i]} ${avg_resp_times[$i]}}" )
	
	sum_of_totRequests=$( awk '{print $1+$2}' <<<"${sum_of_totRequests} ${per_client_totalRequests[$i]}" )
	sum_of_throughput=$( awk '{print $1+$2}' <<<"${sum_of_throughput} ${per_client_throughput[$i]}" )
	sum_of_timeoutrequests=$( awk '{print $1+$2}' <<<"${sum_of_timeoutrequests} ${per_client_timeoutrequests[$i]}" )
	sum_of_errRequets=$( awk '{print $1+$2}' <<<"${sum_of_errRequets} ${per_client_errorRequests[$i]}" )

	
	#sum_of_throughput=$( awk '{ if($3 == 0) {print $1} else { print $1 + ( ($2 * 1000) / $3 ) } }' <<<"${sum_of_throughput} ${success_counts[$i]} ${acc_resp_times[$i]}" )
	#sum_of_throughput=$( awk '{ if($3 == 0) {print $1} else { print $1 + ( ($2 * 1000) / $3 ) } }' <<<"${sum_of_throughput} ${success_counts[$i]} ${loop_times[$i]}" )

done

###################################################
#final sum and outputs considering all the clients:
###################################################

overall_avg_resp_t=0

overall_totRequests=$sum_of_totRequests
overall_throughput=$sum_of_throughput
overall_timeoutrequests=$sum_of_timeoutrequests
overall_sum_of_errRequets=$sum_of_errRequets


overall_avg_resp_t=$( awk '{ if($2 == 0){print 0} else {print $1/$2} }' <<< "${sum_of_resp_time} ${sum_of_success}" )


############################
#Print outputs:
############################

rm -rf temp_files

echo "Number of clients :"$numClients
echo "Overall average response time (in ms) :"$overall_avg_resp_t

echo "Overall requests sent per sec:"$overall_totRequests
echo "Overall throughput per sec:"$overall_throughput
echo "Overall timeout requests per sec:"$overall_timeoutrequests
echo "Overall error requests per sec:"$overall_sum_of_errRequets


