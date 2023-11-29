#!/usr/bin/python3
### Thi script is used to analyse and plot dependng upon server side logged data

import matplotlib.pyplot as plt
from subprocess import run, PIPE
import os

plt.interactive(False)

#Received file name

server_snapshot_log = os.path.expanduser('~/Documents/server_snapshot.log')
server_snapshot_nc_log = os.path.expanduser('~/Documents/server_snapshot_nc.log')
server_service_time_log = os.path.expanduser('~/Documents/serviceTime.log')

# Create lists to store the data
clients = []
averageCpuUtilization = []
averageNumberOfThreads = []
averageServiceTime = []

with open(server_snapshot_log, 'r') as log_file, open(server_snapshot_nc_log, 'r') as nc_file,open(server_service_time_log,'r') as service_time_log:
    log_lines = log_file.readlines()
    nc_lines = nc_file.readlines()
    service_time_line = service_time_log.readlines()

    log_lines = [line.strip() for line in log_lines]
    nc_lines = [line.strip() for line in nc_lines]
    service_time_line = [line.strip() for line in service_time_line]


    log_line_iterator=0
    service_time_line_iterator = 0
    
    #iteration in netcat file
    for n in nc_lines:
        if n.startswith("Number of clients"):
            client_parts = n.split(":")
            clients.append(int(client_parts[1].strip()))
            
            cnt_log_line=0
            cnt_service_time_line=0
            
            avg_cpu=0.0
            avg_thread=0.0
            tot_service_time=0.0
            
            #iteration in log file
            while log_line_iterator<len(log_lines):
                parts = log_lines[log_line_iterator].split()
                # Extract the three parts
                timestamp = parts[0]
                thread = parts[1]
                cpu_utilization = parts[2]

                if timestamp<=prev:
                    avg_thread+=int(thread.strip())
                    avg_cpu+=float(cpu_utilization.strip())
                    cnt_log_line+=1
                else:
                    break
                # print(timestamp,thread,cpu_utilization)
                log_line_iterator+=1

            #iteration in averageQ_log file
            while service_time_line_iterator<len(service_time_line):
                parts = service_time_line[service_time_line_iterator].split()
                # Extract the three parts
                timestamp = parts[0]
                service_time = parts[1]
 
                if timestamp<=prev:
                    tot_service_time +=int(service_time.strip())
                    cnt_service_time_line+=1
                else:
                    break
                # print(timestamp,thread,cpu_utilization)
                service_time_line_iterator+=1
                
                
            averageCpuUtilization.append(avg_cpu/cnt_log_line)
            averageNumberOfThreads.append(avg_thread/cnt_log_line)
            averageServiceTime.append(0 if cnt_service_time_line == 0 else tot_service_time / cnt_service_time_line)
 

        elif n=="exit":
            break
        else:
            prev=n    

    
# Now you can use the collected data to plot the graphs
print("averageCpuUtilization:", averageCpuUtilization)
print("averageNumberOfThreads:", averageNumberOfThreads)
print("averageServiceTimes:",averageServiceTime)


###################
#Plotting graph:#
###################

fig, axs = plt.subplots(2, 2)
fig.suptitle('Autograding server performance analysis : Server side')
 
 
axs[0,0].plot(clients, averageNumberOfThreads, color='magenta', marker='o', markersize=2)
axs[0,0].set(xlabel='Number of clients',ylabel='Thread counts (as an average)')
axs[0,0].set_title("Number of Clients vs Average Thread counts")
 
axs[0,1].plot(clients, averageCpuUtilization, color='red', marker='o', markersize=2)
axs[0,1].set(xlabel='Number of clients', ylabel='Cpu Utilization (in %)')
axs[0,1].set_title("Number of Clients vs Cpu Utilization")
 
axs[1,0].plot(clients, averageServiceTime, color='blue', marker='o', markersize=2)
axs[1,0].set(xlabel='Number of clients', ylabel='Service time (in ms as an average)')
axs[1,0].set_title("Number of Clients vs Average Service time (in ms)")
 
axs[0,0].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[0,1].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[1,0].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[1,1].set_visible(False)
 
fig.tight_layout()
 
plt.show()

os.remove(server_snapshot_log)
os.remove(server_snapshot_nc_log)
os.remove(server_service_time_log)
