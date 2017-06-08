//
// Created by Mrz355 on 07.06.17.
//

#ifndef ANDROI_CHAT_MSG_RECEIVER_H
#define ANDROI_CHAT_MSG_RECEIVER_H

#define MAX_NAME_LEN 108
#define MAX_MESSAGE_LEN 512

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <jni.h>
#include <string>
#include <android/log.h>

void _perror(char* error_msg);
void log(char *msg);

typedef enum msg_type {
    LOGIN, MESSAGE, SUCCESS, FAILURE
} msg_type_t;

typedef struct client {
    int fd;
    char name[MAX_NAME_LEN];
} client_t;

typedef struct msg {
    msg_type_t type;
    int timestamp;
    char name[MAX_NAME_LEN];
    char message[MAX_MESSAGE_LEN];
} msg_t;

#endif //ANDROI_CHAT_MSG_RECEIVER_H
