#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <thread>
#include <queue>
#include <sys/socket.h>
#include <unistd.h> // for close
#include <functional>
#include "accountManager.h"

class TP_submitArgument
{
public:
    std::function<void(int, AccountManager*)> func;
    int sockfd;
    AccountManager* accountManager;

    TP_submitArgument(){};
    TP_submitArgument(std::function<void(int, AccountManager*)> func, int sockfd, AccountManager* accountManager):\
    func(func), sockfd(sockfd), accountManager(accountManager){};
};

// This class manages a thread pool that will process requests
class ThreadPool {
public:
    ThreadPool();
    // The destructor joins all the threads so the program can exit gracefully.
    // This will be executed if there is any exception (e.g. creating the threads)
    ~ThreadPool();
    // This function will be called by the server, every time there is a request
    // that needs to be processed by the thread pool
    void queueWork(TP_submitArgument subArg);
    
private:
    // This condition variable is used for the threads to wait until there is work
    // to do
    std::condition_variable_any workQueueConditionVariable;

    // We store the threads in a vector, so we can later stop them gracefully
    std::vector<std::thread> threads;

    // Mutex to protect workQueue
    std::mutex workQueueMutex;

    // Queue of requests waiting to be processed
    std::queue<TP_submitArgument> workQueue;

    // This will be set to true when the thread pool is shutting down. This tells
    // the threads to stop looping and finish
    bool done;

    // Function used by the threads to grab work from the queue
    void doWork();
    void processRequest(TP_submitArgument subArg);
};

#endif