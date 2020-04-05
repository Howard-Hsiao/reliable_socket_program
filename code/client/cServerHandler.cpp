#include "cServerHandler.h"

void* _serverStarts(void* args)
{
    cServerHandler* argument = (cServerHandler*)args;
    int serverPort = argument->serverPort;
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //bind() of INADDR_ANY does NOT "generate a random IP". It binds the socket to all available interfaces. 
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(serverPort);
    
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    try
    {    
        if(serv_sock < 0)
        {/* connection establishment fails */
            throw runtime_error("[!] Failed to create server socket. \n");
        }

        if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(sockaddr)) < 0)
        {
            string errorMsg = (string("[!] Failed to bind to port") + to_string(serverPort) + ".\n");
            throw runtime_error(errorMsg);
        }

        if (listen(serv_sock, argument->listenNum) < 0)
        {
            throw runtime_error("[!] Failed to listen on socket.\n");
        }
    }
    catch(const std::exception& e)
    {
        cout << e.what() << endl;
        argument->isExit = true;
        argument->isExit_cv.notify_all();
        pthread_exit(nullptr);
        return nullptr;
    }

    argument->isExit_cv.notify_all();
    argument->isServerOpen = true;

    ThreadPool thread_pool;
    while(argument->isServerOpen)
    {
        struct sockaddr_in cli_conn_addr;
        int addrlen = sizeof(sockaddr);
        int conn = accept(serv_sock, (struct sockaddr*)&cli_conn_addr, (socklen_t*)&addrlen);
        if (conn < 0)
        {
            throw runtime_error("[!] Failed to grab connection.\n");
        }
        
        // Add some work to the queue
        TP_submitArgument subArg(worker_func, conn, argument->cli_sock);
        thread_pool.queueWork(subArg);
    }

    pthread_exit(nullptr);
    return nullptr;
}

void worker_func(int src_sockfd, int dst_sock_fd)
{
    string request = yun_recv(src_sockfd);
    close(src_sockfd);
    yun_send(dst_sock_fd, request.c_str(), request.length());
}

void cServerHandler::serverStarts(int serverPort)
{
    this->serverPort = serverPort;
    pthread_create(&this->thread, nullptr, _serverStarts, (void*)(this));
}

void cServerHandler::killThread()
{
    this->isServerOpen = false;
}

void cServerHandler::printInfo()
{
    cout << "serverPort: " << serverPort << endl;
    cout << "isServerOpen: " << isServerOpen << endl;
    cout << "listenNum: " << listenNum << endl;
}
