/*
the code is revised from the "https://ncona.com/2019/05/using-thread-pools-in-cpp/". 
*/
#include "threadPool.h"
#include <thread>             // std::thread
#include <vector>             // std::vector
#include <queue>              // std::queue
#include <mutex>              // std::mutex
#include <condition_variable> // std::condition_variable
using namespace std;

// This class manages a thread pool that will process requests
ThreadPool::ThreadPool() : done(false)
{
    // This returns the number of threads supported by the system. If the
    // function can't figure out this information, it returns 0. 0 is not good,
    // so we create at least 1
    auto numberOfThreads = std::thread::hardware_concurrency();
    if (numberOfThreads == 0)
    {
        numberOfThreads = 1;
    }

    for (unsigned i = 0; i < numberOfThreads; ++i)
    {
        // The threads will execute the private member `doWork`. Note that we need
        // to pass a reference to the function (namespaced with the class name) as
        // the first argument, and the current object as second argument
        threads.push_back(std::thread(&ThreadPool::doWork, this));
    }
}

// The destructor joins all the threads so the program can exit gracefully.
// This will be executed if there is any exception (e.g. creating the threads)
ThreadPool::~ThreadPool()
{
    // So threads know it's time to shut down
    done = true;

    // Wake up all the threads, so they can finish and be joined
    workQueueConditionVariable.notify_all();

    for (std::vector<std::thread>::iterator thread = this->threads.begin();
         thread != this->threads.end(); ++thread)
    {
        if (thread->joinable())
        {
            thread->join();
        }
    }
}

// This function will be called by the server, every time there is a request
// that needs to be processed by the thread pool
void ThreadPool::queueWork(TP_submitArgument subArg)
{
    // Grab the mutex
    std::lock_guard<std::mutex> g(workQueueMutex);

    // Push the request to the queue
    workQueue.push(subArg);

    // Notify one thread that there are requests to process
    workQueueConditionVariable.notify_one();
}
void ThreadPool::doWork()
{
    // Loop while the queue is not destructing
    while (!done)
    {
        TP_submitArgument request;

        // Create a scope, so we don't lock the queue for longer than necessary
        {
            std::unique_lock<std::mutex> g(workQueueMutex);
            workQueueConditionVariable.wait(g, [&] {
                // Only wake up if there are elements in the queue or the program is
                // shutting down
                return !workQueue.empty() || done;
            });

            request = workQueue.front();
            workQueue.pop();
        }

        processRequest(request);
    }
}

void ThreadPool::processRequest(TP_submitArgument subArg)
{
    // Send a message to the connection
    subArg.func(subArg.sockfd, subArg.accountManager);
}