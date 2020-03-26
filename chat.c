#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

void help(){
	printf("\n=============================================================\n");
	printf("help \tDisplay information about available commands\n");
	printf("myip \tDisplay the IP address of this process\n");
	printf("myport \tDisplay the port on which this process is listening for incoming connections\n");
	printf("connect <destination> <port no> \testablishes a new TCP connection\n");
	printf("list \tdisplay connections this process is part of\n");
	printf("terminate <connection id> \t\n");
	printf("send <connection id> <message> \t\n");
	printf("exit \tQuits application\n");
	printf("=============================================================\n");
}
//fix : cross computer myip/myport
int main(int argc, char *argv[]){
	if(argc != 2){
		printf("specify port number!");
		exit(1);
	}
	int port = atoi(argv[1]);
	
	int master_socket, addrlen, new_socket,
		client_socket[30], max_clients = 30,
		activity, i, valread, sd;
	int max_sd;
	struct sockaddr_in address;
	char buffer[1025];
	
	fd_set readfds;
	
	for(i = 0; i < max_clients; i++){
		client_socket[i] = 0;
	}
	
	//socket()
	if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket failed");
		exit(1);
	}
	
	
	//get host ip
	char hostbuffer[256];
	char *IPbuffer;
	struct hostent *host_entry;
	int hostname;
	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
	host_entry = gethostbyname(hostbuffer);
	IPbuffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
	char myIP[sizeof(struct in_addr *)];
	strcpy(myIP, IPbuffer);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	
	//bind()
	if(bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0){
		perror("bind failed");
		exit(1);
	}
	
	//listen()
	if(listen(master_socket, 3) < 0){
		perror("listen");
		exit(1);
	}
	
	addrlen = sizeof(address);
	printf(">> ");
	puts("Waiting for connections...");
	
	
	char arga[3][256];
	char cmd[64];
	char *arg;
	while(1){
		//clear socket set
		FD_ZERO(&readfds);
		//add stdin for command response
		FD_SET(0, &readfds);
		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		//add child sockets to set
		for(i = 0; i < max_clients; i++){
			sd = client_socket[i];
			if(sd > 0){
				FD_SET(sd, &readfds);
			}
			if(sd > max_sd){
				max_sd = sd;
			}
		}
		
		//waits for socket activity
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		
		if((activity < 0) && (errno!=EINTR)){
			printf("select error");
		}

		
		//check for run commands (0 == stdin)
		if(FD_ISSET(0, &readfds)){
			//take inputs
			bzero(cmd, sizeof(cmd));
			int nread;
			if((nread = read(STDIN_FILENO, cmd, sizeof(cmd))) == 0){
				printf("input not read");
			}
			
			//remove newline
			cmd[nread-1] = '\0';
			
			//parse command
			for(i = 0; i < 3; i++){
				bzero(arga[i], sizeof(arga[i]));
			}
			
			argc = 0;
			arg = strtok(cmd, " ");
			while(arg != NULL){
				if(argc == 3)break;
				strcpy(arga[argc], arg);
				argc += 1;
				arg = strtok(NULL, " ");
			}
			
			//user choice
			if(!(strcmp(arga[0], "help"))){
				help();
			}
			
			else if(!(strcmp(arga[0], "myip"))){
				printf("The IP address is %s\n", myIP);
			}
			
			else if(!(strcmp(arga[0], "myport"))){
				printf("The program runs on port number %d\n", port);
			}
			
			else if(!(strcmp(arga[0], "connect"))){
				printf("connecting\n");
				char *ip_to_connect = arga[1];
				int port_to_connect = atoi(arga[2]);
				int clientsockfd;
				//check unique connection
				int exists = 0;
				//check ip:port to connection_socket[0-max] 
				for(i = 0; i < max_clients; i++){
					if(client_socket[i] > 0){
						getpeername(client_socket[i],
							(struct sockaddr *)&address,
							(socklen_t *)&addrlen);
						if(!(strcmp(inet_ntoa(address.sin_addr), ip_to_connect))
							&& ntohs(address.sin_port) == port_to_connect){
								exists = 1;
								printf("Connection to address and port already exists\n");
								break;
							}
					}
				}
				int self = 0;
				if(!strcmp(myIP, ip_to_connect)
					&& port == port_to_connect){
						printf("Cannot connect to self\n");
						self = 1;
					}
				if(exists || self) continue;
				clientsockfd = socket(AF_INET, SOCK_STREAM, 0);
				struct sockaddr_in server_address;
				server_address.sin_family = AF_INET;
				server_address.sin_port = htons(port_to_connect);
				inet_aton(ip_to_connect, &server_address.sin_addr);
				//printf("connecting to %s:%d", ip_to_connect, port_to_connect);
				int connected;
				if((connected = connect(clientsockfd, (struct sockaddr *)&server_address, sizeof(server_address))) < 0){
					printf("Connection Unsuccessful\n");
					
				}
				else{
					printf("Socket %d Connection Success!\n", clientsockfd);
					for(i = 0; i < max_clients; i++){
						if(client_socket[i] == 0){
							client_socket[i] = clientsockfd;
							break;
						}
					}
				}
				
			}
			
			else if(!(strcmp(arga[0], "list"))){
				printf("id: IP address\t\tPort No.\n");
				for(i = 0; i < max_clients; i++){
					//socket filled
					if(client_socket[i] > 0){
						getpeername(client_socket[i],
							(struct sockaddr *)&address,
							(socklen_t *)&addrlen);
						char ip4[20];
						strcpy(ip4, inet_ntoa(address.sin_addr));
						printf("%d:   %s\t\t%d\n",
							i, ip4, ntohs(address.sin_port));
					}
				}
			}
			
			else if(!(strcmp(arga[0], "terminate"))){
				int id = atoi(arga[1]);
				for(i = 0; i < max_clients; i++){
					//remove from list
					if(i == id){
						if(client_socket[i] > 0){
							close(client_socket[i]);
							client_socket[i] = 0;
							printf("socket [id: %d] closed\n", i);
							break;
						}
						else{
							printf("invalid socket\n");
							break;
						}
					}
				}
			}
			
			else if(!(strcmp(arga[0], "send"))){
				char message[256];
				
				int id = atoi(arga[1]);
				
				char* first = arga[2];
				//get full message
				strcpy(message, first);
				strcat(message, " ");
				if(arg != NULL){
					strcat(message, arg);
					strcat(message, " ");
					arg = strtok(NULL, "");
					if(arg != NULL){
						strcat(message, arg);
					}

				}
				
				//printf("%s", message);
				
				for(i = 0; i < max_clients; i++){
					if(id == i){
						if(client_socket[i] > 0){
							send(client_socket[i], message, sizeof(message), 0);
							printf("\nMessage sent to <%d>\n", i);
						}
						else{
							printf("send error. invalid socket\n");
						}
						break;
					}
				}
				
			}
			
			else if(!(strcmp(arga[0], "exit"))){
				exit(1);
			}
			
			else{
				printf("Cannot read input\n");
			}
		}
		
		//incoming new connection
		if(FD_ISSET(master_socket, &readfds)){
			//accept
			if((new_socket = accept(master_socket,
				(struct sockaddr *)&address,
				(socklen_t *)&addrlen)) < 0){
					perror("accept");
					exit(1);
				}
			printf("The connection to peer [%s:%d] is successfully established\n",
				inet_ntoa(address.sin_addr),
				ntohs(address.sin_port));
			
			//add new socket to array of sockets
			for(i = 0; i < max_clients; i++){
				if(client_socket[i] == 0){
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n", i);
					break;
				}
			}
			
		}
		
		//check for disconnects and messages
		for(i = 0; i < max_clients; i++){
			sd = client_socket[i];
			if(FD_ISSET(sd, &readfds) && sd > 0){
				//disconnect
				if((valread = recv(sd, buffer, 1024, 0)) == 0){
					getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
					printf("Host [%s:%d] disconnected\n",
						inet_ntoa(address.sin_addr),
						ntohs(address.sin_port));
					close(sd);
					client_socket[i] = 0;
				}
				//receive message
				else if (valread > 0){
					getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
					printf("\nMessage received from %s\nSender's Port: %d\n",
						inet_ntoa(address.sin_addr),
						ntohs(address.sin_port));
						printf("Message: %s\n", buffer);
				}
				else{
					perror("recv msg");
				}
			}
		}
		
		
	}
	
	
	return 0;
}