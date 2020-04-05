// This header file contains definitions of a number of data types used in system calls. 
// These types are used in the next two include files.
#include <sys/socket.h>
// The header file socket.h includes a number of definitions of structures needed for sockets.
#include <netinet/in.h>
// The header file in.h contains constants and structures needed for internet domain addresses.
#include <arpa/inet.h>  // for "inet_pton".
#include <sys/types.h>
#include <unistd.h> // for close
#include <stdlib.h> //for "atoi"
#include <memory.h>
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm> // for std::count, std::find
#include <stdexcept>
#include <stdlib.h> //for atoi function
#include <regex>
#include <functional>

#include "../yun_library/yun_function.h"
#include "account.h"
#include "accountManager.h"
#include "threadPool.h"
using namespace std;

#ifndef ACCOUNT_CONFIG
#define ACCOUNT_CONFIG
#define defaultMoney 10000
#endif

#ifndef USER_ACTION
#define USER_ACTION

#define REGISTER "register"
#define LOGIN "login"
#define LIST "list"
#define LOGOUT "logout"
#define HELP "help"
#define TRANSFER "transfer"

#endif

void request_thread(int conn, AccountManager* accountManager);
int request_processor(int conn, AccountManager* AccountManager, string& loginAccount);
/*
The desigin of return value is based on the concept of error message. 
@return 0 when conn remains exsitent. 
@return 1 when conn is closed. 
*/

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        throw runtime_error("[!] This program accept exact 1 argument, which is \"port number. \"");
    }

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //bind() of INADDR_ANY does NOT "generate a random IP". It binds the socket to all available interfaces. 
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock < 0)
    {/* connection establishment fails */
        throw runtime_error("[!] Failed to create server socket. \n");
    }

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(sockaddr)) < 0)
    {
        string errorMsg = (string("[!] Failed to bind to port") + string(argv[1])) + ".\n";
        throw runtime_error(errorMsg);
    }
    cout << "Server succuessfully bind to a port. " << endl;
    if (listen(serv_sock, LISTEN_NUM) < 0)
    {
        throw runtime_error("[!] Failed to listen on socket.\n");
    }

    // create key
    fs::path public_key_path = "../public_key/";
    directory_check_create(public_key_path);

    fs::path private_key_path = "../private_key/";
    directory_check_create(private_key_path);
    
    fs::path server_public_key_path  = public_key_path / fs::path("server/");
    directory_check_create(server_public_key_path);
    
    fs::path server_private_key_path = private_key_path / fs::path("server/");
    directory_check_create(server_private_key_path);

    cout << "Please patiently waiting for the process of checking and creating keys. " << endl;

    if((!fs::exists(server_public_key_path / (fs::path)("server_public.pem"))) || \
       (!fs::exists(server_private_key_path / (fs::path)("server_private.pem"))))
    {
        if(!generate_key("server", server_public_key_path, server_private_key_path))
        {
            throw runtime_error("[!] Failed to create keys. \n");
        }
    }

    cout << "The process of checking and creating keys ends. " << endl;

    AccountManager accountManager;
    
    ThreadPool thread_pool;

    while (true)
    {
        // Grab a connection from the queue
        struct sockaddr_in cli_conn_addr;
        int addrlen = sizeof(sockaddr);
        int conn = accept(serv_sock, (struct sockaddr*)&cli_conn_addr, (socklen_t*)&addrlen);
        if (conn < 0)
        {
            throw runtime_error("[!] Failed to grab connection.\n");
        }
        
        // Add some work to the queue
        string request = "Debugging";
        TP_submitArgument subArg(request_thread, conn, &accountManager);
        thread_pool.queueWork(subArg);
    }

    // while(true)
    // {
    //     int addrlen = sizeof(sockaddr);
    //     int conn = accept(serv_sock, (struct sockaddr*)&serv_addr, (socklen_t*)&addrlen);
    //     if (conn < 0) {
    //         throw runtime_error("[!] Failed to grab connection.\n");
    //     }

    //     // get client socket starts
    //     char clientIP[16];
    //     struct sockaddr_in cli_addr;
    //     bzero(&cli_addr, sizeof(cli_addr));
    //     int len = sizeof(cli_addr);
    //     getpeername(conn, (struct sockaddr *) &cli_addr, (socklen_t*)&len);
    //     inet_ntop(AF_INET, &cli_addr.sin_addr, clientIP, sizeof(clientIP));
    //     int cli_port = ntohs(cli_addr.sin_port);
    //     cout << string(clientIP) << ", " << cli_port << endl;

    //     while(true)
    //     {
    //         cout << "------------------------------------------------------------------------" << endl;
    //         request_processor(conn, accountManager);
    //     }
    // }

    close(serv_sock);
    return 0;
}

void request_thread(int conn, AccountManager* accountManager)
{
    string loginAccount;

    int isConnAlive = 0;
    do
    {
        isConnAlive = request_processor(conn, accountManager, loginAccount);
    } while (isConnAlive == 0);
}

int request_processor(int conn, AccountManager* accountManager, string& loginAccount)
{
    string request = yun_recv(conn); 

    cout << "recv_request: " << request;
    if(request[request.length()-1] != '\n')
    {
        cout << endl;
    }

    // register pattern
    string pattern = string(REGISTER) + string("#{1}(\\w+)\n");
    regex register_pattern(pattern);
    // login pattern
    regex login_pattern("(\\w+)#{1}(\\d+)\n");
    
    smatch requestInfo;

    if(regex_search(request, requestInfo, register_pattern))// REGISTER: "#{1}(\\w+)\n"
    {
        if(loginAccount != "")
        {
            string reply = "[!] If you want to register a new account, please log out first. \n";
            yun_send(conn, reply);
            return 0;
        }

        string accountName = requestInfo[1];
        try
        {
            accountManager->registerAccount(accountName);
            string reply = "100 OK\n";
            yun_send(conn, reply);
            return 0;
        }
        catch(invalid_argument)
        {
            string reply = "210 FAIL\n";
            yun_send(conn, reply);
            return 0;
        }
    }
    else if(regex_search(request, requestInfo, login_pattern))// LOGIN: "(\\w+)#{1}(\\d+)\n"
    {
        if(loginAccount != "")
        {
            string reply = "[!] You have already log in an account. \n";
            yun_send(conn, reply);
            return 0;
        }
        // get client socket starts
        char clientIP[16];
        struct sockaddr_in cli_addr;
        bzero(&cli_addr, sizeof(cli_addr));
        int len = sizeof(cli_addr);
        getpeername(conn, (struct sockaddr *) &cli_addr, (socklen_t*)&len);
        inet_ntop(AF_INET, &cli_addr.sin_addr, clientIP, sizeof(clientIP));
        int cli_port = ntohs(cli_addr.sin_port);

        string accountName = requestInfo[1];
        int portNum;
        try
        {
            portNum = stoi(requestInfo[2]);
        }
        catch(invalid_argument) // catch the exception that "stoi" throwing
        {
            string reply = "[!] Your input of port number is not of int type. \n";
            yun_send(conn, reply);
            return 0;
        }
        try
        {
            accountManager->login(accountName, clientIP, portNum);
            loginAccount = accountName;
            string reply = accountManager->getAccountList(loginAccount);
            yun_send(conn, reply);
        }
        catch(invalid_argument e)
        {
            if(!strcmp(e.what(), "invalid portNum"))
            {
                string reply = "[!] Your input of portNum is invalid. \n";
                yun_send(conn, reply);
            }
            else if(!strcmp(e.what(), "invalid IPaddress"))
            {
                string reply = "[!] internal error: cannot validate your IPaddress. \n";
                yun_send(conn, reply);
            }
            else if(!strcmp(e.what(), "nonexistent account"))
            {
                string reply = "220 AUTH_FAIL\n";
                yun_send(conn, reply);
            }
            else if(!strcmp(e.what(), "repetitive login"))
            {
                string reply = "230 REPETITIVE_LOGIN\n";
                yun_send(conn, reply);
            }
        }
        return 0;
    }
    else if(string(request) == string(LIST)+'\n') // LIST
    {
        string reply = accountManager->getAccountList(loginAccount);
        yun_send(conn, reply);
    }
    else if(string(request) == string(LOGOUT)+'\n') // EXIT
    {
        if(loginAccount != "")
        {
            string reply = "Bye\n";
            accountManager->logout(loginAccount);
            yun_send(conn, reply.c_str(), strlen(reply.c_str()));
            close(conn);
            return 1;
        }
        else
        {
            string reply = "[!] You should first login before you can logout. \n";
            yun_send(conn, reply.c_str(), strlen(reply.c_str()));
            return 0;
        }
    }
    else if(count(request.begin(), request.end(), '#') == 2) // transfer; unfinished
    {
        cout << "hihi" << endl;
        regex transfer_pattern("([\\w^#]+)#(\\d+)#([\\w^#]+)");
        smatch transferInfo;
        string src, dst;
        int amount = 0;
        if(regex_search(request, transferInfo, transfer_pattern))
        {
            src = transferInfo[1];
            dst = transferInfo[3];

            try
            {
                amount = stoi(transferInfo[2]);
            }
            catch(invalid_argument)
            {
                string reply = reply + "[!] Your input of amount transferred is not of int type. \n";
            }
        }
        try
        {
            accountManager->transferMoney(src, dst, amount);
        }
        catch(const std::exception& e)
        {
            cout << string(e.what()) << endl;
            string reply = reply + string(e.what());
        }
    }
    else
    {
        cout << "!!!" << request << "!!!" << endl;
        string reply = "[!] Your action is not supported. \n";   
        yun_send(conn, reply);
    }
    return 0;
}