#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
// This header file contains definitions of a number of data types used in system calls. 
// These types are used in the next two include files.
#include <sys/socket.h>
// The header file socket.h includes a number of definitions of structures needed for sockets.
#include <netinet/in.h>
// The header file in.h contains constants and structures needed for internet domain addresses.
//the header file in program_options.hpp contains the usage funciton for parsing argument
#include <arpa/inet.h>  // This is for using function "inet_pton".
#include <unistd.h>
#include <stdlib.h> //for atoi function
#include <stdexcept>
#include <exception>
#include <memory.h>
#include <pthread.h>
#include <mutex>
#include <regex>
#include <condition_variable>
#include "../yun_library/yun_function.h"
#include "cServerHandler.h"
using namespace std;

#ifndef USER_ACTION
#define USER_ACTION

#define REGISTER "register"
#define LOGIN "login"
#define LIST "list"
#define LOGOUT "logout"
#define HELP "help"
#define TRANSFER "transfer"

#endif

#ifndef YUN_NETWORK_CONFIG
#define YUN_NETWORK_CONFIG
#define BUFFER_SIZE 1024
#endif

#ifndef YUN_INTERFACE
#define YUN_INTERFACE
#define commandBlockSep "---\n"
#endif

void printWelcomePage();
void printHelpInfo();
int msg_sender(int cli_sock, string user_action, bool& isExit, char* probable_register_account, 
                char* probable_login_account, cServerHandler& cServer, char* loginAccount);
string msg_receiver(int cli_sock, string user_action, bool& isExit, char* probable_register_account
                    , char* probable_login_account, char* loginAccount);

mutex isExit_mutex;
condition_variable isExit_cv;

// path info starts
fs::path public_key_path = "../public_key/";
fs::path private_key_path = "../private_key/";
fs::path client_public_key_path  = public_key_path / fs::path("client/");
fs::path client_private_key_path = private_key_path / fs::path("client/");

// path info ends

int main(int argc, char** argv)
{
    // setting socket
    char recv_msg[BUFFER_SIZE];
    memset(recv_msg, '\0', BUFFER_SIZE);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    
    int validateAddr;
    if(argc == 3)
    {
        validateAddr = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
            // Convert IPv4 and IPv6 addresses from text to binary form 
        serv_addr.sin_port = htons(atoi(argv[2]));
    }
    else if(argc == 2)
    {
        validateAddr = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
            // Convert IPv4 and IPv6 addresses from text to binary form 
        serv_addr.sin_port = htons(atoi(argv[1]));
    }
    else
    {
        throw runtime_error("[!] The number of argument of the main function is incorrect. ");
    }

    int cli_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(cli_sock < 0)
    {/* connection establishment fails */
        throw runtime_error("[!] Client socket Creation fails. \n");
    } 

    int validateConn = connect(cli_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (validateConn < 0) 
    {
        throw runtime_error("\nConnection Failed \n"); 
    }

    printWelcomePage();
    printHelpInfo();

    // starts change data
    bool isExit = false;

    char probable_register_account[30] = {'\x0'};
    char probable_login_account[30] = {'\x0'};
    char loginAccount[30] = {'\x0'};

    string user_action;
    
    cServerHandler cServer(isExit, isExit_cv, cli_sock);

    // for(int a = 0; a < user_action.length(); a++)
    // {
    //     user_action[a] = tolower(user_action[a]);
    // }

    while(true)
    {
        cout << "Command: ";
        cin >> user_action;
        for(int i = 0; i < user_action.size(); i++)
        {
            user_action[i] = tolower(user_action[i]);
        }

        int successValidator = msg_sender(cli_sock, user_action, isExit, probable_register_account, 
                                          probable_login_account, cServer, loginAccount);

        if(isExit)
        {
            close(cli_sock);
            break;
        }
        
        if(successValidator == 0)
        {
            string reply = msg_receiver(cli_sock, user_action, isExit, probable_register_account, 
                                        probable_login_account, loginAccount);
        }


        if(isExit)
        {
            close(cli_sock);
            break;
        }

        if(!cServer.checkServerOpen() && loginAccount == nullptr)
        {
            cServer.killThread();   
        }
    }
    return 0;
}

void printWelcomePage()
{
    string page = "";
    page = page +
"                                                            _                        " + '\n' +
"                                                            \\`*-.                    " + '\n' +
"                                                             )  _`-.                 " + '\n' +
"                                                            .  : `. .                " + '\n' +
"                                                            : _   '  \\               " + '\n' +
" __      __       .__                                       ; *` _.   `*-._          " + '\n' +
"/  \\    /  \\ ____ |  |   ____  ____   _____   ____          `-.-'          `-.       " + '\n' +
"\\   \\/\\/   // __ \\|  | _/ ___\\/  _ \\ /     \\_/ __ \\           ;       `       `.     " + '\n' +
" \\        /\\  ___/|  |_\\  \\__(  <_> )  Y Y  \\  ___/           :.       .        \\    " + '\n' +
"  \\__/\\  /  \\___  >____/\\___  >____/|__|_|  /\\___  >          . \\  .   :   .-'   .   " + '\n' +
"       \\/       \\/          \\/            \\/     \\/           '  `+.;  ;  '      :   " + '\n' +
"                                                              :  '  |    ;       ;-. " + '\n' +
"          This is a Yun's socket program! :D                  ; '   : :`-:     _.`* ;" + '\n' +
"                                                      [bug] .*' /  .*' ; .*`- +'  `*'" + '\n' +
"                                                            `*-*   `*-*  `*-*'       " + '\n' +
"                                                           Art by Blazej Kozlowski   " + '\n' +
"=====================================================================================" + '\n';
    cout << page;
}



void printHelpInfo()
{
    cout << "What action do you want to do?" << endl;
    cout << "(This program support both uppercase and lowercase input. )" << endl;
    cout << "-> press \"" << REGISTER << "\": Register a new account. " << endl;
    cout << "-> press \"" << LOGIN << "\": Login an account. " << endl;
    cout << "-> press \"" << LIST << "\": List user info. " << endl;
    cout << "-> press \"" << LOGOUT << "\":Logout the account and exit the program. " << endl;
    cout << "-> press \"" << HELP << "\": Get help info. " << endl;
    cout << "-> press \"" << TRANSFER << "\": Transfer money to other client. " << endl;
    cout << "-----" << endl;
}

string msg_receiver(int cli_sock, string user_action, bool& isExit, char* probable_register_account
                    , char* probable_login_account, char* loginAccount)
{
    cout << endl << "Output: " << endl;
    string reply = yun_recv(cli_sock);

    if(reply == "100 OK\n")
    {
        directory_check_create(public_key_path);
        directory_check_create(private_key_path);
        directory_check_create(client_public_key_path);
        directory_check_create(client_private_key_path);

        string client_public_file  = *probable_register_account + "_public.pem";
        string client_private_file = *probable_register_account + "_private.pem";

        if((!fs::exists(client_public_key_path  / (fs::path)(client_public_file.c_str()))) || \
           (!fs::exists(client_private_key_path / (fs::path)(client_private_file.c_str()))))
        {
            if(!generate_key(string(probable_register_account), client_public_key_path, client_private_key_path))
            {
                throw runtime_error("[!] Failed to create keys. \n");
            }
        }

        cout << "Register success" << endl;
    }
    else if(reply == "210 FAIL\n")
    {
        cout << "[!] The name you requested has been already registered, " << 
            "please use another name to register. " << endl;
    }
    else if(reply == "220 AUTH_FAIL\n")
    {
        if(user_action == LOGIN)
        {
            cout << "[!] Login fails: " << endl;
            cout << "The reason can be: " << endl;
            cout << " -> The account you want to login does not exist. " << endl;
            cout << " -> The port number is invalid. " << endl;
        }
        else if(user_action == LIST)
        {
            cout << "[!] You should first login and then you can see the account info. " << endl;    
        }
        else if(user_action == LOGOUT)
        {
            cout << "[!] You should first login and then you can logout. " << endl;
        }
        else
        {
            cout << "[!] Please do the valid action. " << endl;
        }    
    }
    else if(reply == "230 REPETITIVE_LOGIN\n")
    {
        cout << "[!] The account has been logined somewhere. " << endl;
    }

    else if(reply == "Bye\n")
    {
        cout << "Connection close." << endl;
        cout << "Bye~" << endl;
        isExit = true;
    }
    else if(user_action == LIST)
    {
        cout << reply;
    }
    else if(user_action == LOGIN)
    {
        strcpy(loginAccount, probable_login_account);
        loginAccount[strlen(probable_login_account)] = '\x00';

        cout << reply;
    }
    else
        cout << reply;

    cout << "************************************" << endl;
    cout << endl;
    return reply;
}

int msg_sender(int cli_sock, string user_action, bool& isExit, char* probable_register_account, 
                char* probable_login_account, cServerHandler& cServer, char* loginAccount)
/*
@return 0 when succeeds
@return 1 when fails
*/
{
    string send_msg;
    if(user_action == REGISTER)
    {
        string accountName;
        cout << "Please input your desire account name: ";
        cin >> accountName;

        send_msg += REGISTER;
        send_msg += '#';
        send_msg += accountName;

        strcpy(probable_register_account, accountName.c_str());
        probable_register_account[accountName.length()] = '\x00';
    }
    else if(user_action == LOGIN)
    {
        string accountName;
        cout << "Please input your account name: ";
        cin >> accountName;
        string portNum;
        //should try to see if port Num is str, then what happen
        cout <<  "Please input your port number: ";
        cin >> portNum;
        
        try
        {
            int testPortNum = stoi(portNum);
        }
        catch (invalid_argument)
        {
            cout << "[!] The port number should be an integer. " << endl;
            cout << commandBlockSep;
            return 1;
        }

        send_msg += accountName;
        send_msg += '#';
        send_msg += portNum;

        strcpy(probable_login_account, accountName.c_str());
        probable_login_account[accountName.length()] = '\x00';

        cServer.serverStarts(stoi(portNum));
        unique_lock<mutex> mLock(isExit_mutex);
        isExit_cv.wait( mLock );

        if(isExit)
        {
            if(loginAccount != nullptr)
                yun_send(cli_sock, (string(LOGOUT)+'\n').c_str(), strlen(LOGOUT)+1);
            return 1;
        }

    }
    else if(user_action == LIST)
    {
        send_msg = LIST;
    }
    else if(user_action == LOGOUT)
    {
        send_msg = LOGOUT;
    }
    else if(user_action == HELP)
    {
        cout << endl;
        printHelpInfo();
        return 1;
    }
    else if(user_action == TRANSFER)
    {
        if(loginAccount == nullptr)
        {
            cout << "[!] Before you tranfer money to someone, you should first log in. " << endl;
            cout << commandBlockSep;
            return 1;
        }
        
        int payment = -1;
        string payee;
        cout << "How much do you want to pay? ";
        
        string error_msg = "[!] Please input valid amount of money. \n";
        try
        {
            cin >> payment;
            if(payment <= 0)
            {
                throw invalid_argument(error_msg.c_str());
            }
        }
        catch(exception)
        {
            while(true)
            {
                try
                {
                    string junk;
                    cin >> junk;
                    cin >> payment;
                    if(payment <= 0)
                    {
                        cout << error_msg;
                        throw invalid_argument(error_msg.c_str());
                    }
                    break;
                }
                catch(const std::exception& e)
                {
                    cout << e.what();
                }
            }
        }
        cout << "Who do you want to transfer money to? ";
        cin >> payee;

        if(payee == string(loginAccount))
        {
            cout << "[!] You cannot pay yourself. \n";
            return 1;
        }

        send_msg = string(loginAccount);
        send_msg += "#";
        send_msg += to_string(payment);
        send_msg += "#";
        send_msg += payee;

        // setting socket
        string listInfo = getListInfo(cli_sock);

        string transfer_pattern_str = payee;
        transfer_pattern_str += '#';
        transfer_pattern_str += "(\\d+\\.\\d+\\.\\d+\\.\\d+)";//can be better
        transfer_pattern_str += "#(\\d+)\n";

        regex transfer_pattern(transfer_pattern_str.c_str());
        smatch transferInfo;
        
        string payee_IP_str;
        string payee_portNum_str;

        if(regex_search(listInfo, transferInfo, transfer_pattern))
        {   
            payee_IP_str = transferInfo[1];
            payee_portNum_str = transferInfo[2];
        }
        else
        {
            cout << "[!] " << payee << " is not online now. \n";
            return 1;
        }
        
        char recv_msg[BUFFER_SIZE];
        memset(recv_msg, '\0', BUFFER_SIZE);

        struct sockaddr_in serv_addr;
        bzero(&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        

        int validateAddr;
        validateAddr = inet_pton(AF_INET, payee_IP_str.c_str(), &serv_addr.sin_addr);
            // Convert IPv4 and IPv6 addresses from text to binary form 
        serv_addr.sin_port = htons(stoi(payee_portNum_str));


        int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(sock_fd < 0)
        {/* connection establishment fails */
            throw runtime_error("[!] Client socket Creation fails. \n");
        } 

        int validateConn = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if (validateConn < 0) 
        {
            throw runtime_error("\nConnection Failed \n"); 
        }

        string request = string(loginAccount);
        request += "#";
        request += to_string(payment);
        request += "#";
        request += payee;

        yun_send(sock_fd, request.c_str(), request.length());

        close(sock_fd);

        return 1; 
    }
    else
    {
        cout << "[!] Detect syntax error, please input the correct command. " << endl;
        cout << commandBlockSep;
        return 1;
    }

    send_msg = send_msg + '\n';
    yun_send(cli_sock, send_msg.c_str(), strlen(send_msg.c_str()));
    return 0;
}