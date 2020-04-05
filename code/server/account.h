#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
using namespace std;

#ifndef ACCOUNT_CONFIG
#define ACCOUNT_CONFIG
#define defaultMoney 10000
#endif

class Account
{
private:
    string accountName;
    int accountBalance;
    string IP_address;
    int portNum;
    bool loginFlag;

public:
    friend class AccountManager;
    friend bool operator==(string str, const Account& account);
    friend bool operator==(const Account& account, string str);
    Account(string accountName, int portNum=-1, string IP_address="", int accountBalance=defaultMoney, bool loginFlag = false):
        accountName(accountName), portNum(portNum), IP_address(IP_address), accountBalance(accountBalance), loginFlag(loginFlag){};
};

#endif