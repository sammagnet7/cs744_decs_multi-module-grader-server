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

# Read the log file line by line and process the data
# with open('log.txt', 'r') as log_file:
#     lines = log_file.readlines()
#     print(lines)
#     i = 0
#     cnt=0
#     avg_cpu=0.0
#     avg_thread=0.0
#     while i < len(lines):
#         line = lines[i].strip()
#         if line == "exit":
#             print(line)
#             break
#         if line.startswith("Number of clients:"):
#             print("count: ",cnt)
#             print("avg_thread: ",avg_thread)
#             print("avg_cpu: ",avg_cpu)

#             parts = line.split(":")
#             clients.append(int(parts[1].strip()))
		
#             averageCpuUtilization.append(avg_cpu/cnt)
#             averageNumberOfThreads.append(avg_thread/cnt)
            
#             avg_thread=0.0
#             avg_cpu=0.0
#             cnt=0
            
#             i+=1
#         else:
#              print(lines[i].strip())
#              avg_thread+=int(lines[i].strip())
#              i+=1
             
#              print(lines[i].strip())
#              avg_cpu+=float(lines[i].strip())
#              i+=1
             
#              cnt+=1

with open(server_snapshot_log, 'r') as log_file, open(server_snapshot_nc_log, 'r') as nc_file:
    log_lines = log_file.readlines()
    nc_lines = nc_file.readlines()

    log_lines = [line.strip() for line in log_lines]
    nc_lines = [line.strip() for line in nc_lines]

    j=0
    for n in nc_lines:
        if n.startswith("Number of clients"):
            client_parts = n.split(":")
            clients.append(int(client_parts[1].strip()))
            cnt=0
            avg_cpu=0.0
            avg_thread=0.0
            while j<len(log_lines):
                parts = log_lines[j].split()
                # Extract the three parts
                timestamp = parts[0]
                thread = parts[1]
                cpu_utilization = parts[2]

                if timestamp<=prev:
                    avg_thread+=int(thread.strip())
                    avg_cpu+=float(cpu_utilization.strip())
                    cnt+=1
                else:
                    break
                # print(timestamp,thread,cpu_utilization)
                j+=1
            averageCpuUtilization.append(avg_cpu/cnt)
            averageNumberOfThreads.append(avg_thread/cnt)

        elif n=="exit":
            break
        else:
            prev=n    

    
# Now you can use the collected data to plot the graphs
print("averageCpuUtilization:", averageCpuUtilization)
print("averageNumberOfThreads:", averageNumberOfThreads)




###################
#Plotting graph:2 #
###################
fig, axs = plt.subplots(1, 2)
fig.suptitle('Autograding server performance analysis : Server side')


axs[0].plot(clients, averageNumberOfThreads, color='magenta', marker='o')
axs[0].set(xlabel='Number of clients',ylabel='Average Thread counts')
axs[0].set_title("Number of Clients vs Average Thread counts")

axs[1].plot(clients, averageCpuUtilization, color='black', marker='o')
axs[1].set(xlabel='Number of clients',ylabel='Cpu Utilization')
axs[1].set_title("Number of Clients vs Cpu Utilization")



axs[0].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[1].grid(color = 'green', linestyle = '--', linewidth = 0.5)

fig.tight_layout()

plt.show()
# #plt.savefig("graph.png", bbox_inches='tight')

os.remove(server_snapshot_log)
os.remove(server_snapshot_nc_log)


