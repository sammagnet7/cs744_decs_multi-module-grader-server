#!/Users/soumikdutta/miniconda3/bin/python3
import matplotlib.pyplot as plt
from subprocess import run, PIPE
import os

plt.interactive(False)

#Received file name

server_snapshot_log = os.path.expanduser('~/Documents/server_snapshot.log')
server_snapshot_nc_log = os.path.expanduser('~/Documents/server_snapshot_nc.log')

# Create lists to store the data
clients = []
averageCpuUtilization = []
averageNumberOfThreads = []


with open(server_snapshot_log, 'r') as log_file, open(server_snapshot_nc_log, 'r') as nc_file:
    log_lines = log_file.readlines()
    nc_lines = nc_file.readlines()

    log_lines = [line.strip() for line in log_lines]
    nc_lines = [line.strip() for line in nc_lines]


    log_line_iterator=0
    
    #iteration in netcat file
    for n in nc_lines:
        if n.startswith("Number of clients"):
            client_parts = n.split(":")
            clients.append(int(client_parts[1].strip()))
            
            cnt_log_line=0
            
            avg_cpu=0.0
            avg_thread=0.0
            
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
                
                
            averageCpuUtilization.append(avg_cpu/cnt_log_line)
            averageNumberOfThreads.append(avg_thread/cnt_log_line)

        elif n=="exit":
            break
        else:
            prev=n    

    
# Now you can use the collected data to plot the graphs
print("averageCpuUtilization:", averageCpuUtilization)
print("averageNumberOfThreads:", averageNumberOfThreads)



###################
#Plotting graph:#
###################


fig, axs = plt.subplots(2,sharex=True, sharey=True)
fig.suptitle("Number of clients")
axs[0].plot(clients,averageNumberOfThreads, color='magenta', marker='o')
axs[1].plot(clients,averageCpuUtilization, color='black', marker='o')

axs[0].set(ylabel='Average Thread counts')
axs[1].set(ylabel='Cpu Utilization')



axs[0].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[1].grid(color = 'green', linestyle = '--', linewidth = 0.5)

fig.tight_layout()

plt.show()
#plt.savefig("graph.png", bbox_inches='tight')

os.remove(server_snapshot_log)
os.remove(server_snapshot_nc_log)