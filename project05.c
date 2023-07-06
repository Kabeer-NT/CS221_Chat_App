#include "project05.h"

//Function to setup poll giving in the pollfds, user_list_le and file descriptor
void setup_poll(struct pollfd *polls, int i, int fd){
	polls[i].fd = fd;
	polls[i].events = POLLIN;
	polls[i].revents = 0;
}

//Function to check record given a users list and a user, len here is the number of records in user list
int check_record(struct user *lst, struct user *u, int len){
	for(int i = 0; i < len; i++){
		if(!strncmp(u->name, lst[i].name, strlen(u->name))){ //Check if name is same
			if(!strncmp(u->status, lst[i].status, strlen(u->status))){ //check if status is same
				return -1; //code for if name and status is same
			} else {
				return i; //code for if name is same but status is different then return the user_list_len of the record
			}
		}
	}
	return -2; //code for if the name doesn't exist at all
}

//Function to read presence given a struct user and a fd
int main(int argc, char **argv) {
	
	if(argc != 3){
		printf("Usage: ./project05 <Your_Name> <Port>");
		exit(-1);
	}

	struct user user_list[MAX_POLLS];
	int user_list_len = 0;

	bool eof = false;
    	struct pollfd my_polls[NUM_POLLS];
    	int num_polls = 0;
    	int num_readable = 0;
    	char ch;
	int counter = 0;

	//Setup STDIN_FILENO and UDP Polls
    	setup_poll(my_polls, 0, STDIN_FILENO);
    	setup_poll(my_polls, 1, setup_presence());	
	setup_poll(my_polls, 2, setup_tcp());
	num_polls += 3; //Increment by 3 for all flags

	write_presence(my_polls[1].fd, 0); //Initial presence message
   
	char buffer[50]; //Buff that everything will be stored in
    	while (!eof) { 
		num_readable = poll(my_polls, num_polls, TIMEOUT);
	        if (num_readable > 0) {
			for(int i = 0; i < num_polls; i++){
				if(!my_polls[i].revents & POLLIN){
					continue;
				} else {
					switch(i){
					        case STDIN_FILENO:
						    	if(read(num_readable, &ch, 1) == 0){
							    	eof = true; //end loop
							    	write_presence(my_polls[1].fd,1); //write offline presence
							    	break;
						    	}
						    	strncat(buffer, &ch, 1);
							if(ch == '\n'){ //If you hit enter then write and reset buffer
								write_message(buffer, user_list, user_list_len);
								memset(buffer, 0, sizeof(buffer));
								continue;
							}
							break;
						case UDP_BROADCAST:
							read_presence(my_polls[i].fd, user_list, &user_list_len); //Read all people online
							if(counter > 20){ //Periodically Write presence messages(2000 ms)
								write_presence(my_polls[1].fd, 0);
								counter = 0;
								break;
							}
							break;
						case TCP_LISTENER:
							int chat_fd = accept(my_polls[2].fd, NULL, NULL);
							if(chat_fd == -1){
								perror("accept failed");
								exit(-1);
							}
							setup_poll(my_polls, num_polls, chat_fd);
							num_polls++;
							break;
						default:
							listen_tcp(my_polls[i].fd, user_list, user_list_len);
							break;
					}
				}	
			}					
	        } else if (num_readable == -1) {
	            	perror("poll failed");
	           	exit(-1);
	        } else if (num_readable == 0){
			counter++;
		    }
	}
	//Closing the poll fds
	for(int i = 0; i < num_polls; i++){
		close(my_polls[i].fd);
	}

}
