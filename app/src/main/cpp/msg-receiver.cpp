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

int socket_fd;

char username[MAX_NAME_LEN];

JavaVM* javaVM = NULL;
JNIEnv* env=0;
jclass globalClass;
jobject globalInstance;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void * reserved)  {
    javaVM = vm; return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)  {
    javaVM = NULL;
}


void *message_receiver(void *argv) {
    int res = javaVM->AttachCurrentThread(&env, NULL);
    jclass cls = env->GetObjectClass(globalInstance);
    msg_t msg;
    int s = 1;
    while(s) {

        log("readddinnggg");
        if(read(socket_fd,&msg,sizeof(msg)) == -1) {
            _perror("message_receiver: read");
        }
        log("alreeady reaaaad");
        jstring message = env->NewStringUTF(msg.message);
        jmethodID method = env->GetMethodID(cls, "newMessage", "(Ljava/lang/String;)V");
        env->CallVoidMethod(globalInstance, method, message);
    }
    javaVM->DetachCurrentThread();
    pthread_exit(NULL);
    return NULL;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_randar_androichat_MainActivity_listenCoroutine(JNIEnv *env, jobject instance) {
    env->GetJavaVM(&javaVM);
    jclass cls = env->GetObjectClass(instance);
    globalClass = (jclass) env->NewGlobalRef(cls);
    globalInstance = env->NewGlobalRef(instance);

    pthread_t tid;
    pthread_attr_t attr;

    int pt = pthread_create(&tid,NULL,&message_receiver,NULL);

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
JNIEXPORT jstring JNICALL
Java_com_randar_androichat_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT jint JNICALL
Java_com_randar_androichat_MainActivity_connectToServer(JNIEnv *env, jobject instance) {
    log("i'm here in c");
    if((socket_fd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
        _perror("socket");
        return -1;
    }

    char *addr = "10.205.11.113";
    uint16_t port = 36000;

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(port);
    in_addr_t bin_addr = inet_addr(addr);
    in_addr.sin_addr.s_addr = bin_addr;


    if(connect(socket_fd, (const struct sockaddr *) &in_addr, sizeof(in_addr)) == -1) {
        _perror("connect");
        return -1;
    }

    return 0;
}

JNIEXPORT jint JNICALL
Java_com_randar_androichat_MainActivity_login(JNIEnv *env, jobject instance, jstring jusername) {
    const char *_username = env->GetStringUTFChars(jusername, 0);

    int res = 0;

    strcpy(username,_username); // save our username

    msg_t msg;
    msg.type = LOGIN;
    strcpy(msg.name,_username);
    // try to login
    if(write(socket_fd, &msg, sizeof(msg)) == -1) {
        _perror("login: write");
        res = -1;
    }
    // wait for response
    if(read((socket_fd), &msg, sizeof(msg)) == -1) {
        _perror("login: read");
        res = -1;
    }

    if(msg.type == SUCCESS) {
        log("logged in successfully");
        log("logged in successfully");
    } else {
        log("failed to login");
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
Java_com_randar_androichat_MainActivity_logout(JNIEnv *env, jobject instance) {
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