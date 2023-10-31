#!/Users/soumikdutta/miniconda3/bin/python3

##Give the server ip and port to connect to get the Server side statistics
server_ip='10.157.3.213'
server_port = 12345

from subprocess import run, PIPE
import matplotlib.pyplot as plt
import time
import socket

minClients = int( input( 'Enter count of minimum number(included) of clients: ' ) )
maxClients = int( input( 'Enter count of maximum number(included) of clients: ' ) )
steps = int( input( 'Enter increasing steps: ' ) )

clients = []

clients_vs_reqSent = []
clients_vs_throughput = []
clients_vs_timeout = []
clients_vs_errReq = []

clients_vs_responseTime = []

try: 
	client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client_socket.settimeout(1)
	client_socket.connect((server_ip, server_port))
except Exception as e:
	print(e)
	print("SERVER side STATS ANALYZER is down. Also check server Ip and Port. Meanwhile ignoring and proceeding without that...")


for count in range(minClients,maxClients+steps,steps):

	output = run(['./loadtest.sh', str(count),'20','0.5', '3'], stdout=PIPE).stdout.splitlines()

	clients.append(int(count))
	clients_vs_responseTime.append( float( str( output[2] ).split( ':' )[1].rstrip("'") ) )

	clients_vs_reqSent.append( float( str( output[3] ).split( ':' )[1].rstrip("'") ) )
	clients_vs_throughput.append( float( str( output[4] ).split( ':' )[1].rstrip("'") ) )
	clients_vs_timeout.append( float( str( output[5] ).split( ':' )[1].rstrip("'") ) )
	clients_vs_errReq.append( float( str( output[6] ).split( ':' )[1].rstrip("'") ) )

	time.sleep(2)
	msg = f"Number of clients: {count}\n"

	try: 
		client_socket.send(msg.encode())
	except Exception as e:
		pass
msg = "exit\n"

try: 
	client_socket.send(msg.encode())
	client_socket.close()
except Exception as e:
	pass

print(clients_vs_responseTime)	
print(clients_vs_reqSent)
print(clients_vs_throughput)
print(clients_vs_timeout)
print(clients_vs_errReq)


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

