////////////////////////////////////////////////////////
//  The function "generate_key()" is revised from the code from 
//  https://www.codepool.biz/how-to-use-openssl-generate-rsa-keys-cc.html
////////////////////////////////////////////////////////

#ifndef YUN_FUNCTION_H
#define YUN_FUNCTION_H

#include <sys/socket.h>
#include <stdexcept>
#include <cstring>
#include <string>
#include <fstream>
#include <openssl/rsa.h> // need to add flag "-l ssl -l crypto" when compiling
#include <openssl/pem.h> // need to add flag "-l ssl -l crypto" when compiling
#include <filesystem> // need to used g++9 and set the flag "-std=c++17" and "-lstdc++fs"
#include <iostream> // for debugging 

using namespace std;
namespace fs = std::filesystem;

#ifndef YUN_NETWORK_SETTING
#define YUN_NETWORK_SETTING

#define BUFFER_SIZE 1024
#define LISTEN_NUM 10

#endif

#ifndef TAG_MEANING
#define TAG_MEANING
// the measurement is bytes
#define TAG_B_SIZE 1
#define TAG_H_SIZE 2
#define TAG_L_SIZE 4

#define TAG_B_CAPACITY 256
#define TAG_H_CAPACITY 65536
#define TAG_L_CAPACITY 4294967296
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

typedef char BYTE;

const BYTE* int_to_header(unsigned data_size);
int getTagSize(const char& tag);
int yun_send(int socket_fd, const char* send_msg, int msg_size);
int yun_send(int socket_fd, string send_msg);
/*
return 0 when success;
return 1 when failure;
*/

const char* yun_recv(int cli_sock);
inline void free_all(BIO* bp_public, BIO* bp_private, RSA* r, BIGNUM* bne);
bool generate_key(string key_owner, fs::path public_key_path, fs::path private_key_path);
void directory_check_create(fs::path target);

const char* getListInfo(int cli_sock);

#endif