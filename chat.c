#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
//select()
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

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

int main(int argc, char **argv){
	
	if(argc != 2){
		printf("specify port number!");
		exit(1);
	}
	
	int port = atoi(argv[1]);
	char msg[256];
	bzero(&msg, sizeof(msg));
	
	//select variables
	int opt = 1;
	int master_socket, addrlen, new_socket, client_socket[20], max_clients = 20, activity, i, valread, sd;
	int max_sd;
	struct sockaddr_in address;
	
	char buffer[1025];
	
	fd_set readfds;

	for (i = 0; i < max_clients; i++){
		client_socket[i] = 0;
	}

	if( (master_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("server: socket");
		exit(1);
	}
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0){
			perror("setsockopt");
			exit(1);
		}
	bzero(&address, sizeof(address));
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	
	
	if( (bind(master_socket, (struct sockaddr *)&address, sizeof(address))) < 0){
		close(master_socket);
		perror("server: bind");
		exit(1);
	}
	
	if( listen(master_socket, 10) < 0){
		perror("listen");
		exit(1);
	}
	addrlen = sizeof(address);
	
	char hostbuffer[256];
	char *IPbuffer;
	struct hostent *host_entry;
	int hostname;
	
	hostname = gethostname(hostbuffer, sizeof(hostbuffer));
	host_entry = gethostbyname(hostbuffer);
	IPbuffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
	
	
	printf("Server: waiting for connections... \n");
	
	
	//end of setting up socket
	
	
	char cmd[20];
	char *arg;
	while(1){
		printf("asdf");
		int clientsockfd;
		struct sockaddr_in addrto;
		int connected;
		char *ip_to_connect;
		int port_to_connect;
		
		//accept incoming requests--
		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		
		for(i = 0; i < max_clients; i++){
			sd = client_socket[i];
			
			if(sd > 0){
				FD_SET(sd, &readfds);
			}
			
			if(sd >  max_sd){
				max_sd = sd;
			}
		}
		struct timeval tv;
		tv.tv_sec = 10;
		activity = select(max_sd + 1, &readfds, NULL, NULL, &tv.tv_sec);
		
		if((activity < 0) && (errno!=EINTR)){
			printf("select error");
		}
		
		if(FD_ISSET(master_socket, &readfds)){
			if((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen) < 0)){
				perror("accept");
				exit(1);
			}
			
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n",
			new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
		
			if( send(new_socket, msg, strlen(msg), 0) != strlen(msg) ){
				perror("send");
			}
		
			puts("Welcome message sent successfully");
		
			for (i = 0; i < max_clients; i++){
				if( client_socket[i] == 0){
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n");
					break;
				}
			}
		}
		for(i = 0; i < max_clients; i++){
			sd = client_socket[i];
			if(FD_ISSET(sd, &readfds)){
				if((valread = read(sd, buffer, 1024)) == 0){
					getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
					printf("Host disconnected, ip %s, port %d \n",
						inet_ntoa(address.sin_addr), ntohs(address.sin_port));
					close(sd);
					client_socket[i] = 0;
				}
			}
			else{
				buffer[valread] = '\0';
				send(sd, buffer, strlen(buffer), 0);
			}
		}
		
		
		/* get client's ip
		char theirbuffer[256];
		char *theirIPbuffer;
		struct hostent *theirhost_entry;
		int theirhostname;
		theirhostname = gethostname(theirbuffer, sizeof(theirbuffer));
		host_entry = gethostbyname(theirhostname);
		theirIPbuffer = inet_ntoa(*((struct in_addr *)theirhost_entry->h_addr_list[0]));
		printf("Server: got connections from %s\n", theirIPbuffer);
		*/
		
		//take inputs
		printf(">> ");
		scanf("%s", cmd);
		
		argc = 0;
		arg = strtok(cmd, " ");
		while(arg){
			strcpy(argv[argc], arg);
			argc += 1;
			arg = strtok(NULL, " ");
		}
		
		//user choice
		if(!(strcmp(argv[0], "help"))){
			help();
		}
		
		else if(!(strcmp(argv[0], "myip"))){
			printf("The IP address is %s\n", IPbuffer);
		}
		
		else if(!(strcmp(argv[0], "myport"))){
			printf("The program runs on port number %d\n", port);
		}
		
		else if(!(strcmp(argv[0], "connect"))){
			*ip_to_connect = argv[1];
			port_to_connect = argv[2];
			printf("connecting to %s:%d", ip_to_connect, port_to_connect);
			clientsockfd = socket(AF_INET, SOCK_STREAM, 0);
			addrto.sin_family = AF_INET;
			addrto.sin_port = port_to_connect;
			addrto.sin_addr.s_addr = inet_addr(ip_to_connect);
			printf("connecting to %s:%d", ip_to_connect, port_to_connect);
			if((connected = connect(clientsockfd, (struct sockaddr *)&addrto, sizeof(addrto))) < 0){
				printf("Connection Unsuccessful");
			}
			
			if(connected){
				
			}
			
			
			
		}
		
		else if(!(strcmp(argv[0], "list"))){
			
		}
		
		else if(!(strcmp(argv[0], "terminate"))){
			
		}
		
		else if(!(strcmp(argv[0], "send"))){
			printf("Enter a message: ");
			fgets(msg, 256, stdin);
			send(clientsockfd, msg, strlen(msg), 0);
		}
		
		else if(!(strcmp(argv[0], "exit"))){
			exit(1);
		}
		
		else{
			printf("Cannot read input\n");
		}
		
	}



}