# Systems_IITB
Git repo link:  https://github.com/sammagnet7/Systems_IITB.git

## client_side
cd Systems_IITB

# Make a zip of server_side folder and send to server side machine using scp command 	(scp hostname@ip:~/Folder)
tar zcvf server_side.tar server_side
ssh labuser@10.130.154.66 			(ssh into 4 tabs to run 3 servers in 3 tabs and 1 snapshot script)
scp server_side.tar labuser@10.130.154.66:~/SoumikD/cs744

cd client_side
make
cd ../perf_test
change server_ip in plot_client_stats.py 	(ip address of the server)
change serverip_port in loadtest.sh 		(ip address of server and port of submission server)


## server_side

#Extract the server_side code on the server using the below command
tar xvf server_side.tar 
make 		(Run make on the server side)
# set client_machine_path in server_snapshot.sh script - 

## Now for performance analysis--

# Server side:
./submission_server 8080	(1st tab)

./query_server 9090		(2nd tab)

./grader_server			(3rd tab)

./server_snapshot		(4th tab)


#Client side:
python3 plot_client_stats.py

#line
