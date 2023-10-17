#!/Users/soumikdutta/miniconda3/bin/python3
from subprocess import run, PIPE
import matplotlib.pyplot as plt
import time
plt.interactive(False)


minClients = int( input( 'Enter count of minimum number of clients: ' ) )
maxClients = int( input( 'Enter count of maximum number of clients: ' ) )
steps = int( input( 'Enter increasing steps: ' ) )

clients = []

clients_vs_reqSent = []
clients_vs_throughput = []
clients_vs_timeout = []
clients_vs_errReq = []

clients_vs_responseTime = []

for count in range(minClients,maxClients+1,steps):

	output = run(['./loadtest.sh', str(count),'5','1', '0.1'], stdout=PIPE).stdout.splitlines()

	clients.append(int(count))
	clients_vs_responseTime.append( float( str( output[1] ).split( ':' )[1].rstrip("'") ) )

	clients_vs_reqSent.append( float( str( output[2] ).split( ':' )[1].rstrip("'") ) )
	clients_vs_throughput.append( float( str( output[3] ).split( ':' )[1].rstrip("'") ) )
	clients_vs_timeout.append( float( str( output[4] ).split( ':' )[1].rstrip("'") ) )
	clients_vs_errReq.append( float( str( output[5] ).split( ':' )[1].rstrip("'") ) )

	#time.sleep(0.2)

print(clients_vs_responseTime)	
print(clients_vs_reqSent)
print(clients_vs_throughput)
print(clients_vs_timeout)
print(clients_vs_errReq)

###################
#Plotting graph:1 #
###################

# fig, (ax1, ax2) = plt.subplots(1, 2)
# fig.suptitle('Autograding server performance analysis')


# ax1.plot(clients, clients_vs_throughput, color='blue', marker='o')
# ax1.set(xlabel='Number of clients', ylabel='Throughput')
# ax1.set_title("Number of Clients vs Throughput")


# ax2.plot(clients, clients_vs_responseTime,color='red', marker='o')
# ax2.set(xlabel='Number of clients', ylabel='Response time')
# ax2.set_title("Number of Clients vs Response time")

# ax1.grid(color = 'green', linestyle = '--', linewidth = 0.5)
# ax2.grid(color = 'green', linestyle = '--', linewidth = 0.5)

# plt.show()
#plt.savefig("graph.png", bbox_inches='tight')


###################
#Plotting graph:2 #
###################
fig, axs = plt.subplots(2, 3)
fig.suptitle('Autograding server performance analysis')


axs[0,0].plot(clients, clients_vs_timeout, color='magenta', marker='o')
axs[0,0].set(xlabel='Number of clients',ylabel='Timeouts')
axs[0,0].set_title("Number of Clients vs Timeouts")

axs[0,2].plot(clients, clients_vs_errReq, color='yellow', marker='o')
axs[0,2].set(xlabel='Number of clients', ylabel='Error Requests')
axs[0,2].set_title("Number of Clients vs Error Requests")


axs[1,2].plot(clients, clients_vs_responseTime,color='red', marker='o')
axs[1,2].set(xlabel='Number of clients', ylabel='Response time')
axs[1,2].set_title("Number of Clients vs Response time")

axs[1,1].plot(clients, clients_vs_reqSent, color='cyan', marker='o')
axs[1,1].set(xlabel='Number of clients', ylabel='Requests sent')
axs[1,1].set_title("Number of Clients vs Requests sent")

axs[1,0].plot(clients, clients_vs_throughput, color='blue', marker='o')
axs[1,0].set(xlabel='Number of clients', ylabel='Throughput')
axs[1,0].set_title("Number of Clients vs Throughput(goodput)")


axs[0,0].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[0,1].set_visible(False)
axs[0,2].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[1,0].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[1,1].grid(color = 'green', linestyle = '--', linewidth = 0.5)
axs[1,2].grid(color = 'green', linestyle = '--', linewidth = 0.5)

fig.tight_layout()

plt.show()
#plt.savefig("graph.png", bbox_inches='tight')

