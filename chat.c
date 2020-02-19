#include <stdio.h>

int main(int argc, char *argv[]){
	
	argc = 0;
	arg = strtok(cmd, " ");
	while(arg){
		strcpy(argv[argc], arg);
		argc += 1;
		
		arg = strtok(NULL, " ");
	}
	if(strcmp(argv[0], "help")){
		
	}
	if(strcmp(argv[0], "myip")){
		
	}
	if(strcmp(argv[0], "myport")){
		
	}
	if(strcmp(argv[0], "connect")){
		
	}
	if(strcmp(argv[0], "list")){
		
	}
	if(strcmp(argv[0], "terminate")){
		
	}
	if(strcmp(argv[0], "send")){
		
	}
	if(strcmp(argv[0], "exit")){
		
	}
}