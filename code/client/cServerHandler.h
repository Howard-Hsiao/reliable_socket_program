#ifndef C_SERVER_HANDLER
#define C_SERVER_HANDLER

#include <sys/socket.h>
// The header file socket.h includes a number of definitions of structures needed for sockets.
#include <netinet/in.h>
// The header file in.h contains constants and structures needed for internet domain addresses.
#include <arpa/inet.h>  // for "inet_pton".
#include <sys/types.h>
#include <unistd.h> // for close
#include <pthread.h>
#include <stdexcept>
#include <strings.h>
#include <condition_variable>
#include <mutex>
#include <iostream> // used for debugging
#include "cli_threadPool.h"
#include "../yun_library/yun_function.h"
// #include "threadPool.h"
using namespace std;


void worker_func(int src_sockfd, int dst_sockfd);

class cServerHandlerArg
{
public: 
    int& serverPort;
    bool& isServerOpen;
    int listenNum;
    cServerHandlerArg(int& serverPort, bool& isServerOpen, int& listenNum)
                        :serverPort(serverPort), isServerOpen(isServerOpen), listenNum(listenNum){};
};

class cServerHandler
{
private:
    pthread_t thread;
    int serverPort;
    bool isServerOpen;
    int listenNum; 
    bool& isExit;
    condition_variable& isExit_cv;
    int cli_sock;

public:
    friend void* _serverStarts(void* args);
    cServerHandler(bool& isExit, condition_variable& isExit_cv, int cli_sock)
        :isServerOpen(false), listenNum(10), isExit(isExit), isExit_cv(isExit_cv), cli_sock(cli_sock){};
    void serverStarts(int serverPort);
    void killThread();
    bool checkServerOpen(){return this->isServerOpen;};
    void printInfo();
};

void* _serverStarts(void* args);

#endif