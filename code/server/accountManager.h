#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H


#include <vector>
#include <stdexcept>
#include <string>
#include <algorithm>
#include "account.h"
#include <iostream>
#include <mutex>
#include <regex>

using namespace std;

class AccountManager
{
private:
    vector<Account> accountInfo;
    mutex protector;
public:
    void transferMoney(string src, string dst, int amount);
    void registerAccount(string accountName);
    void login(string accountName, string IP_address, int portNum);
    void logout(string accountName);
    string getAccountList(string accountName);
};

#endif