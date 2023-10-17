#!/Users/soumikdutta/miniconda3/bin/python3
from subprocess import run, PIPE
import matplotlib.pyplot as plt
import time
plt.interactive(False)


minClients = int( input( 'Enter count of minimum number of clients: ' ) )
maxClients = int( input( 'Enter count of maximum number of clients: ' ) )
steps = int( input( 'Enter increasing steps: ' ) )

clients = []
clients_vs_throughput = []
clients_vs_responseTime = []

for count in range(minClients,maxClients+1,steps):

	output = run(["./loadtest.sh", str(count),"5","1"], stdout=PIPE).stdout.splitlines()

	clients.append(count)
	clients_vs_throughput.append( float( str( output[2] ).split( ':' )[1].rstrip("'") ) )
	clients_vs_responseTime.append( float( str( output[1] ).split( ':' )[1].rstrip("'") ) )
	time.sleep(0.2)

	
print(clients_vs_throughput)
print(clients_vs_responseTime)

###############
#Plotting graph
###############

fig, (ax1, ax2) = plt.subplots(1, 2)
fig.suptitle('Autograding server performance analysis')


ax1.plot(clients, clients_vs_throughput, color='blue', marker='o')
ax1.set(xlabel='Number of clients', ylabel='Throughput')
ax1.set_title("Number of Clients vs Throughput")


ax2.plot(clients, clients_vs_responseTime,color='red', marker='o')
ax2.set(xlabel='Number of clients', ylabel='Response time')
ax2.set_title("Number of Clients vs Response time")

ax1.grid(color = 'green', linestyle = '--', linewidth = 0.5)
ax2.grid(color = 'green', linestyle = '--', linewidth = 0.5)

plt.show()
#plt.savefig("graph.png", bbox_inches='tight')



