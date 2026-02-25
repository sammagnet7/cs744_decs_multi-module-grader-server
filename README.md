# Systems_IITB

[Git repo link](https://github.com/sammagnet7/Systems_IITB.git)

## client_side

```bash
cd Systems_IITB
```

Make a zip of server_side folder and send to server side machine using scp command: 	```scp hostname@ip:~/Folder```

```bash
tar zcvf server_side.tar server_side
scp server_side.tar labuser@10.130.154.66:~/SoumikD/cs744

ssh labuser@10.130.154.66 			

# ssh into 4 tabs to run 3 servers in 3 tabs and 1 snapshot script

cd client_side
make

cd ../perf_test

#change serverip_port in loadtest.sh 		(ip address of server and port of submission server)

#change server_ip in plot_client_stats.py 	(ip address of the server)
```

## server_side
Extract the server_side code on the server using the below command:

```bash
tar xvf server_side.tar

cd server_side
make 		#(Run make on the server side)

#set client_machine_path in server_snapshot.sh script


## Server side:
./submission_server 8080 3	(1st tab)
./query_server 9090 3		(2nd tab)
./grader_server	6		(3rd tab)
./server_snapshot		(4th tab)
```

## Client side:
```bash
python3 plot_client_stats.py
```

## Server side:
After grading completion -> Close Grader server with cntr+c
Move to the tab where server_snapshot is running -> Give the user password of the client_side machine

## Client side
```bash
python3 plot_server_stats.py
```

> For more info checkout: ./Docs

