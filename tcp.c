#include "project05.h"

//Function to setup the tcp listener
int setup_tcp(){
	struct addrinfo hints;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo *rp;
        int rc = getaddrinfo(NULL,"8082", &hints, &rp);
        if(rc !=0){
                fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rc));
                exit(-1);
        }
        int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(fd == -1){
                perror("socket");
                exit(-1);
        }
	int enable = 1;
        if(setsockopt(fd,SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) != 0){
                perror("setsockopt reuseaddr");
                exit(-1);
        }
	if(ioctl(fd,FIONBIO,&enable) != 0){
		perror("ioctl failed");
		exit(-1);
	} 
	if(bind(fd,rp->ai_addr, rp->ai_addrlen) != 0) {
                perror("bind");
                exit(-1);
        }

	if(listen(fd,MAX_POLLS) != 0){
		perror("listen Failed");
		exit(-1);
	}

	freeaddrinfo(rp);
        return fd;	
}

//Function to get a name from a hostname in the list of struct user
char* lookup_user(char* hostname, struct user *lst, int len){
	for(int i = 0; i < len; i++){
		if(!strncmp(hostname, lst[i].host, strlen(hostname))){
			return lst[i].name;
		}
	}
	perror("Record not found");
	exit(-1);
}

//Function to read the message from a given connection
void listen_tcp(int lfd, struct user *lst, int len){
	struct sockaddr_storage peer;
	socklen_t peer_len = sizeof(peer);

	char service[NI_MAXSERV]; //buffer that port is to be copied into
	char buf[BUF_SIZE]; //Buffer that the message is to be copied into
	
	struct user u;

	int num_recvd = recv(lfd,buf,BUF_SIZE,0); //Number of stuff recieved 

	if(num_recvd == -1){			//Error in recieving
		perror("Recieve Faied");
		exit(-1);
	} else if (num_recvd == 0){		//If user is offline
		return;
	}
	if(getpeername(lfd,(struct sockaddr*) &peer, &peer_len) != 0){
		perror("getpeername Error");
		exit(-1);
	}
	if(getnameinfo((struct sockaddr*) &peer, sizeof(peer), u.host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV) != 0){

		perror("getnameinfo failed");
		exit(-1);
	}
	char *sent = lookup_user(u.host,lst,len);
	printf("NEW MESSAGE from %s: %s \n",sent,buf);	
}

void write_message(char *message, struct user *lst, int len){
	char user[64];
	char msg[64];
	char unfiltered[BUF_SIZE];
	char host[NI_MAXHOST];
	char port[NI_MAXSERV];
	memset(user, 0, sizeof(user));
	memset(msg, 0, sizeof(msg));
	memset(unfiltered, 0, sizeof(unfiltered));
	memset(host, 0, sizeof(host));
	memset(port, 0, sizeof(port));
	
	if(!(message[0] == '@')){
		fprintf(stderr,"Please Mention someone using an @\n");
		return;
	}

	sscanf(message, "@%s",unfiltered); //The unfiltered message without the @

	for(int i = 0; i < strlen(unfiltered); i++){
		if(unfiltered[i] == ':'){
			if(i == 0){ //Check to see if the first thing is nothing i.e @:<something>
				fprintf(stderr,"Please Enter the name of an online user\n");
				return;
			}
			if(i == strlen(unfiltered) - 1 ){ //Check to see if the last thing is a : i.e @<something>:
				fprintf(stderr,"Please Write a Valid Message\n");
				return;
			}
		}
	}
	
	strncpy(user,strtok(unfiltered,":"),sizeof(user)); //Copy the stuff before the : to user
	strncpy(msg,strtok(NULL,":"),sizeof(msg)); //Copy the stuff after the : to user

	if(strlen(user) == 0) {
		fprintf(stderr,"Please Enter the name of an online user\n");
		exit(-1);
	}
	if(strlen(msg) == 0) {
		fprintf(stderr,"Please write something\n");
		exit(-1);
	}
	
	struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

	for(int i = 0; i < len; i++){ //Iterate through the list of all users and find the port and host for the same name
		if(!strncmp(user, lst[i].name, sizeof(user))){
			strncpy(port, lst[i].port, sizeof(port));
			strncpy(host, lst[i].host, sizeof(host));
			break;	
		}
	}

	struct addrinfo *rp;
	int rc = getaddrinfo(host, port, &hints, &rp);
        if(rc != 0){
                fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rc));
                exit(-1);
        }
        int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(fd == -1){
                perror("socket\n");
                exit(-1);
        }
	if(connect(fd,rp->ai_addr,rp->ai_addrlen) != 0){
		perror("Connection Failed\n");
		return;
	}

	int sent = send(fd,msg,sizeof(msg),0);
	if(sent == -1){
		perror("sending failed");
		return;
	} else if (sent == 0){
		perror("Nothing Was sent");
		return;
	} else {
		printf("Message sent\n");
	}
}
