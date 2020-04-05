#include "account.h"

bool operator==(string str, const Account& account)
{
    return account.accountName == str;
}

bool operator==(const Account& account, string str)
{
    return account.accountName == str;
}