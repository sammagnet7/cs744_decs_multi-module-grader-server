# Systems_IITB
Git repo link:  https://github.com/sammagnet7/Systems_IITB.git


#Zip the server side code and send it to the server
set the "client_machine_path" in "/server_side/server_snapshot.sh" (ex- hostname@IP:~/Documents) so that server can send the log files to client
zip -r server_side.zip server_side
scp server_side.zip labuser@10.130.154.66:~/SoumikD/cs744/Autograder_server_V2

#ssh server to unzip server code 
ssh labuser@10.130.154.66
cd ~/SoumikD/cs744/Autograder_server_V2
unzip server_side.zip
cd server_side/

#open another tab to do ssh again to connect to server:
ssh labuser@10.130.154.66
cd ~/SoumikD/cs744/Autograder_server_V2
cd server_side/

#Performance Test on client side
Checkout to Autograder_branch_version to be tested
git branch -a	 		(to check all branch names)
git checkout Autograder_V2	(checkout to version2)

Client side: cd client_side/
	     make
	  
Server side: make

Client Side: 	cd ../perf_test/
	     	make
	     	change serverip_port in loadtest.sh 		(if required)
	     	change server_ip in plot_client_stats.py 	(if required)
	     		
python3 plot_client_stats.py

########## JUST GIVE BELOW MENTIONED VALUES, DON'T HIT ENTER ################
Enter minimum clients:  1
Enter maximum clients:  100
Enter steps	     :  10


#Start the server
./server <port_no>

#Run script "./server_snapshot.sh" 
./server_snapshot.sh

#Just after running the above script hit enter at client side "python3 plot_client_stats.py"

#Just after the graph is generated on client side stop the server and enter client machine password in the seecond tab where "server_snapshot.sh" is running 

#On client side run-
python3 plot_server_stats.py 


#line
