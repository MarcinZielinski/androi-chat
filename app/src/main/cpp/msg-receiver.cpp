//
// Created by Mrz355 on 07.06.17.
//

#include <jni.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include "msg-receiver.h"

#define APPNAME "androi-chat"

int socket_fd = -1;
pthread_t tid;
pthread_t connection_tid;
char username[MAX_NAME_LEN];

JavaVM* javaVM = NULL;
JNIEnv* env=0;
jclass globalClass;
jobject globalInstance;

int coroutineStarted;
int connectionCoroutineStarted;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void * reserved)  {
    javaVM = vm; return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)  {
    javaVM = NULL;
}

void terminate_thread(int signum) {
    javaVM->DetachCurrentThread();
    pthread_exit(0);
}

void *message_receiver(void *argv) {
    int res = javaVM->AttachCurrentThread(&env, NULL);
    jclass cls = env->GetObjectClass(globalInstance);
    msg_no_type_t msg;
    msg_type_t type;

    struct sigaction sigact;
    sigact.sa_flags = 0;
    sigemptyset(&(sigact.sa_mask));
    sigact.sa_handler = terminate_thread;
    sigaction(SIGUSR2,&sigact,NULL);

    int s = 1;
    while(s) {

        log("readddinnggg");
        if(read(socket_fd,&type,sizeof(type)) == -1) {
            _perror("message_receiver: read type");
            coroutineStarted = 0;
            return NULL;
        }

        switch(type) {

            case MESSAGE: {
                if (read(socket_fd, &msg, sizeof(msg)) == -1) {
                    _perror("message_receiver: read");
                    coroutineStarted = 0;
                    return NULL;
                }
                jstring message = env->NewStringUTF(msg.message);
                jstring _username = env->NewStringUTF(msg.name);
                jmethodID method = env->GetMethodID(cls, "newMessage",
                                                    "(Ljava/lang/String;Ljava/lang/String;)V");
                env->CallVoidMethod(globalInstance, method, _username, message);
                break;
            }
            case PING: {
                type = PONG;
                if (write(socket_fd, &type, sizeof(type)) == -1) {
                    _perror("message_receiver: write");
                    coroutineStarted = 0;
                    return NULL;
                }
                log("pong sent");
                break;
            }
            default: {
                _perror("switch wrong type");
            }
        }
        log("alreeady reaaaad");
    }
    javaVM->DetachCurrentThread();
    pthread_exit(NULL);
    return NULL;
}

void thread_failure_exit(jclass cls) {
    connectionCoroutineStarted = 0;

    jmethodID method = env->GetMethodID(cls, "connectionRefused", "()V");
    env->CallVoidMethod(globalInstance, method);

    javaVM->DetachCurrentThread();
    pthread_exit(NULL);
}

void *connectToServer(void *args) {
    int res = javaVM->AttachCurrentThread(&env, NULL);
    jclass cls = env->GetObjectClass(globalInstance);

    struct sigaction sigact;
    sigact.sa_flags = 0;
    sigemptyset(&(sigact.sa_mask));
    sigact.sa_handler = terminate_thread;
    sigaction(SIGUSR2,&sigact,NULL);

    if((socket_fd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
        _perror("socket");
        thread_failure_exit(cls);
        return (void *) -1;
    }

    int flags;
    if ((flags = fcntl(socket_fd, F_GETFL, 0)) == -1) {
        perror ("fcntl");
        thread_failure_exit(cls);
        return (void *) -1;
    }

    flags |= O_NONBLOCK;

    if (fcntl(socket_fd, F_SETFL, flags) == -1) {
        perror ("fcntl");
        thread_failure_exit(cls);
        return (void *) -1;
    }

    //char *addr = "83.242.74.12";
    char *addr = "192.168.43.209";
    uint16_t port = 36000;

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(port);
    in_addr_t bin_addr = inet_addr(addr);
    in_addr.sin_addr.s_addr = bin_addr;

    if(connect(socket_fd, (const struct sockaddr *) &in_addr, sizeof(in_addr)) == -1) {
        if (errno == EINPROGRESS) {
            struct timeval stTv;
            stTv.tv_sec = 5;
            stTv.tv_usec = 0;
            fd_set write_fd;
            FD_ZERO(&write_fd);
            FD_SET(socket_fd,&write_fd);
            socklen_t iLength = sizeof(int);
            int iValOpt;
            if (select ((socket_fd + 1) , &write_fd,&write_fd,NULL,&stTv) > 0) {
                if(getsockopt(socket_fd,SOL_SOCKET,SO_ERROR,(void*)(&iValOpt),&iLength) < 0) {
                    _perror("getsockopt");
                    thread_failure_exit(cls);
                    return (void *) -1;
                }
                if (iValOpt != 0) {
                    _perror("iValOpt");
                    thread_failure_exit(cls);
                    return (void *) -1;
                }
            } else {
                _perror("select timeout");
                thread_failure_exit(cls);
                return (void *) -1;
            }
        } else {
            _perror("connect");
            thread_failure_exit(cls);
            return (void *) -1;
        }
    }

    if ((flags = fcntl(socket_fd, F_GETFL, 0)) == -1) {
        perror ("fcntl");
        thread_failure_exit(cls);
        return (void *) -1;
    }

    flags &= (~ O_NONBLOCK);

    if (fcntl(socket_fd, F_SETFL, flags) == -1) {
        perror ("fcntl");
        thread_failure_exit(cls);
        return (void *) -1;
    }

    jmethodID method = env->GetMethodID(cls, "connectionEstablished", "()V");
    env->CallVoidMethod(globalInstance, method);

    connectionCoroutineStarted = 0;
    javaVM->DetachCurrentThread();
    pthread_exit(NULL);
    return NULL;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_randar_androichat_MainActivity_listenCoroutine(JNIEnv *env, jobject instance) {
    log("listen coroutine started");
    env->GetJavaVM(&javaVM);
    jclass cls = env->GetObjectClass(instance);
    globalClass = (jclass) env->NewGlobalRef(cls);
    globalInstance = env->NewGlobalRef(instance);

    pthread_attr_t attr;

    int pt = pthread_create(&tid,NULL,&message_receiver,NULL);
    if(pt == 0) {
        coroutineStarted = 1;
    }
    //if(pt != 0) {
    //    _perror("pthread_create");
    //}

    return 0;
}

void _perror(char* error_msg) {
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s: %d %s\n", error_msg, errno, strerror(errno));
}

void log(char *msg) {
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s\n", msg);
}


extern "C" {

JNIEXPORT jint JNICALL
Java_com_randar_androichat_LoginActivity_connectToServer(JNIEnv *_env, jobject instance) {
    env = _env;
    env->GetJavaVM(&javaVM);
    jclass cls = env->GetObjectClass(instance);
    globalClass = (jclass) env->NewGlobalRef(cls);
    globalInstance = env->NewGlobalRef(instance);

    if(pthread_create(&connection_tid,NULL,connectToServer,NULL) == 0) {
        connectionCoroutineStarted = 1;
        return 0;
    } else {
        return -1;
    }
}

JNIEXPORT jint JNICALL
Java_com_randar_androichat_LoginActivity_login(JNIEnv *env, jobject instance, jstring jusername) {
    const char *_username = env->GetStringUTFChars(jusername, 0);
    log("login cpp method started");

    int res = 0;

    strcpy(username,_username); // save our username

    msg_t msg;
    msg_type_t type;
    msg.type = LOGIN;
    strcpy(msg.name,_username);
    // try to login
    if(write(socket_fd, &msg, sizeof(msg)) == -1) {
        _perror("login: write type");
        res = -1;
    }

    //if(write(socket_fd, &msg, sizeof(msg)) == -1) {
    //    _perror("login: write");
    //    res = -1;
    //}
    // wait for response
    if(read((socket_fd), &type, sizeof(type)) == -1) {
        _perror("login: read");
        res = -1;
    }

    if(type == SUCCESS) {
        log("logged in successfully");
    } else {
        log("failed to login, type:");
        char buf[sizeof(int)];
        sprintf(buf,"%d",type);
        log(buf);
        res = -1;
    }

    env->ReleaseStringUTFChars(jusername, _username);
    return res;
}

JNIEXPORT jint JNICALL
Java_com_randar_androichat_MainActivity_sendMessage(JNIEnv *env, jobject instance, jstring jmessage) {
    const char* message = env->GetStringUTFChars(jmessage,0);
    int res = 0;

    msg_t msg;
    msg.type = MESSAGE;
//    msg_type_t type = MESSAGE;
    strcpy(msg.name,username);
    strcpy(msg.message,message);

    if(write(socket_fd, &msg, sizeof(msg)) == -1) {
        _perror("sendMessage: write");
        res = -1;
    }

    env->ReleaseStringUTFChars(jmessage, message);
    return res;
}


JNIEXPORT jint JNICALL
Java_com_randar_androichat_LoginActivity_logout(JNIEnv *env, jobject instance) {
    if(coroutineStarted == 1) {
        log("killing listen coroutine");
        if(pthread_kill(tid, SIGUSR2)) {
            _perror("pthread_kill listen");
        }
        coroutineStarted = 0;
    }
    if(connectionCoroutineStarted == 1) {
        log("killing connection coroutine");
    //    if(pthread_kill(connection_tid, SIGUSR2)) {
    //        _perror("pthread_kill connection");
    //    }
        connectionCoroutineStarted = 0;
    }
    int res = 0;
    if (socket_fd != -1) {
        if(shutdown(socket_fd, SHUT_RDWR) == -1) {
            _perror("logout: shutdown");
            res = 1;
        }
        if(close(socket_fd) == -1) {
            _perror("logout: close");
            res = 1;
        }
    }
    return res;
}
}