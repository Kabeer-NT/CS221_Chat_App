#ifndef _PROJECT05_H
#define _PROJECT05_H

#include <poll.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>


#define TIMEOUT 10
#define BUF_SIZE 128
#define MAX_POLLS 64
#define POLL_STDIN 0
#define NUM_POLLS 50
#define STDIN_FILENO 0
#define UDP_BROADCAST 1
#define TCP_LISTENER 2

struct user{
	char status[64];
	char name[64];
	char port[64];
	char host[64];
};

static char *user_input;
static char *port_input;

void poll_setup(struct pollfd *poll);

//Prototype for UDP Functions
int setup_presence();
void write_presence(int fd, int status);
void read_presence(int fd, struct user *lst, int *count);

int check_record(struct user *lst, struct user *u, int len);

//Prototype for TCP Function
int setup_tcp();
void listen_tcp(int lfd, struct user *lst, int len);
void write_message(char *message, struct user *lst, int len);
#endif
