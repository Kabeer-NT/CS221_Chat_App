#include "project05.h"

//Function to setup_presence, returns fd which is used as the pole fd of UDP
int setup_presence(){
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        struct addrinfo *rp;
        int rc = getaddrinfo(NULL, "8221", &hints, &rp);
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
        if(setsockopt(fd,SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) != 0){
                perror("setsockopt broadcast");
                exit(-1);
        }
        if(setsockopt(fd,SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable)) != 0){
                perror("setsockopt reuseport");
                exit(-1);
        }
        if(bind(fd, rp->ai_addr, rp->ai_addrlen) != 0) {
                perror("bind");
                exit(-1);
        }
        freeaddrinfo(rp);
        return fd;
}

//Function to write presence, paramater is fd->file descriptor and status-> 0 for online 1 for offline
void write_presence(int fd, int status){
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, "10.10.13.255", &addr.sin_addr);
        addr.sin_port = htons(8221);
	char *buff;
        if(status == 0){
                buff = "knthakur online 8082";
        } else {
                buff = "knthakur offline 8082";
        }
        if(sendto(fd, buff, BUF_SIZE, 0, (struct sockaddr*)&addr, sizeof(addr)) == -1){
		perror("write presence error");
		return;
	}
}

//Function to read presence given a struct user and a fd
void read_presence(int fd, struct user *lst, int *count){
        struct user u;
        struct sockaddr_storage stg;
        char buf[BUF_SIZE] = "";
        socklen_t stg_len = sizeof(stg);
	char service[NI_MAXSERV];	
        
	recvfrom(fd, buf, 128, 0, (struct sockaddr*)&stg, &stg_len);
	getnameinfo((struct sockaddr*) &stg, stg_len, u.host, sizeof(u.host), service, NI_MAXSERV, NI_NUMERICSERV);

        sscanf(buf,"%s %s %s",u.name, u.status, u.port);
        int result = check_record(lst,&u,*count); //Check to see if the record exists or not
        if(result == -1){
                //Do nothing
                return;
        } else if(result == -2) {
                //add record and increment count
                lst[*count] = u;
                (*count)++;
                printf("%s is %s on port %s\n",u.name,u.status,u.port);
        } else {
                //Update the record status i.e turn online to offline and visa versa
                strncpy(lst[result].status, u.status, strlen(lst[result].status));
                printf("%s is %s on port %s\n",u.name, u.status, u.port);
        }
}
