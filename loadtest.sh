#!/bin/bash
#set -x #for bash debugging
if [ $# -ne 3 ]; then
	echo "Usage: $0 <numClients> <loopNum> <sleepTimeSeconds>"
	exit
fi

numClients=$1
loopNum=$2
sleepTimeSeconds=$3

counter=$numClients

for (( i = 0; i < $counter; i++ )); do
	./submit 10.157.3.213:8080 test/source_P.cpp $loopNum $sleepTimeSeconds > output_$i.txt 2>&1 &
done

wait

#echo "==================== done ====================="


###################################
#storing each file data into array:
###################################

declare -a avg_resp_times
declare -a success_counts
declare -a per_client_throughput
#declare -a acc_resp_times
#declare -a loop_times

for (( i = 0; i < $counter; i++ )); do


	avg_resp_times[$i]=$(cat output_$i.txt | awk '/Average response time/ {print $0}' | awk -F: '{print $2;}')

	success_counts[$i]=$(cat output_$i.txt | awk '/Number of successful responses/ {print $0}' | awk -F: '{print $2;}')

	per_client_throughput[$i]=$( cat output_$i.txt | awk '/Individual client throughput/ {print $0}' | awk -F: '{print $2;}'  )


	#acc_resp_times[$i]=$(cat output_$i.txt | awk '/Accumulated response time/ {print $0}' | awk -F: '{print $2;}')
	#loop_times[$i]=$(cat output_$i.txt | awk '/Time taken for completing client loop/ {print $0}' | awk -F: '{print $2;}')

done

#####################################
#calculating outputs for each clients:
#####################################

sum_of_success=0
sum_of_resp_time=0
sum_of_throughput=0


for (( i = 0; i < $counter; i++ )); do

	sum_of_success=$( awk '{print $1+$2}' <<<"${sum_of_success} ${success_counts[$i]}" )
	sum_of_resp_time=$( awk '{print $1+($2*$3)}' <<<"${sum_of_resp_time} ${success_counts[$i]} ${avg_resp_times[$i]}}" )
	sum_of_throughput=$( awk '{print $1+$2}' <<<"${sum_of_throughput} ${per_client_throughput[$i]}" )
	
	#sum_of_throughput=$( awk '{ if($3 == 0) {print $1} else { print $1 + ( ($2 * 1000) / $3 ) } }' <<<"${sum_of_throughput} ${success_counts[$i]} ${acc_resp_times[$i]}" )
	#sum_of_throughput=$( awk '{ if($3 == 0) {print $1} else { print $1 + ( ($2 * 1000) / $3 ) } }' <<<"${sum_of_throughput} ${success_counts[$i]} ${loop_times[$i]}" )

done

###################################################
#final sum and outputs considering all the clients:
###################################################

overall_avg_resp_t=0
overall_throughput=$sum_of_throughput

overall_avg_resp_t=$(awk '{ if($1 == 0){print 0} else {print $2/$1} }' <<< "${sum_of_success} ${sum_of_resp_time}" )


############################
#Print outputs:
############################
rm output_*
echo "Number of clients :"$numClients
echo "Average response time (in ms) :"$overall_avg_resp_t
echo "Overall throughput :"$overall_throughput



