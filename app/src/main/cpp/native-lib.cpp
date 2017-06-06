#include <jni.h>
#include <string>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <android/log.h>


#define APPNAME "androi-chat"


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
        int socket_fd;

        log("Am i here man?");

        if((socket_fd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
            _perror("socket");
            return -1;
        }

        char *addr = "192.168.1.126";
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
}
