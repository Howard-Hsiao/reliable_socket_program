#include "accountManager.h"

void AccountManager::transferMoney(string src, string dst, int amount)
{
    lock_guard<std::mutex>(this->protector);
    auto payer = find(this->accountInfo.begin(), this->accountInfo.end(), src);

    if (payer == this->accountInfo.end())
    {
        throw invalid_argument("[!] The payer does not exist. ");
    }

    auto payee = find(this->accountInfo.begin(), this->accountInfo.end(), dst);
    if (payee == this->accountInfo.end())
    {
        throw invalid_argument("[!] The payee does not exist. ");
    }

    if(amount > payer->accountBalance)
    {
        throw invalid_argument("[!] The payer does not have enough money. ");
    }

    payer->accountBalance -= amount;
    payee->accountBalance += amount;
}


void AccountManager::registerAccount(string accountName)
{
    lock_guard<std::mutex>(this->protector);
    auto target = find(this->accountInfo.begin(), this->accountInfo.end(), accountName);
    if (target != this->accountInfo.end())
    {
        throw invalid_argument("[!] the account name you want to registerd has been used. ");
    }
    else
        this->accountInfo.push_back(Account(accountName));
}


void AccountManager::login(string accountName, string IP_address, int portNum)
{
    // validate IP_address and portNum
    if(portNum < 1024 || portNum > 65535)
    {
        throw invalid_argument("invalid portNum");
    }


    regex IPvalidator("([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3})");
    smatch IPresult;
    if(!regex_match(IP_address, IPresult, IPvalidator))
    {
        throw invalid_argument("invalid IPaddress");
    }
    for(int a = 1; a <= 4; a++)
    {
        if(stoi(IPresult[a]) < 0 || stoi(IPresult[a]) > 255)
            throw invalid_argument("invalid IPaddress");
    }

    lock_guard<std::mutex>(this->protector);
    auto target = find(this->accountInfo.begin(), this->accountInfo.end(), accountName);
    if (target != this->accountInfo.end())
    {
        if(target->loginFlag)
        {
            throw invalid_argument("repetitive login");
        }
        target->portNum = portNum;
        target->IP_address = IP_address;
        target->loginFlag = true;
    }
    else
    {
        throw invalid_argument("nonexistent account");
    }
}

string AccountManager::getAccountList(string accountName)
{
    string reply;
    auto target = find(this->accountInfo.begin(), this->accountInfo.end(), accountName);
    reply = reply + to_string(target->accountBalance) + string("\n");

    int onlineNum = count_if (this->accountInfo.begin(), this->accountInfo.end(),
                    [](Account member) { return member.loginFlag; });

    reply = reply +  "number of accounts online: " + to_string(onlineNum) + string("\n");
    for (vector<Account>::iterator member = this->accountInfo.begin() ; member != this->accountInfo.end();
         member++)
    {
        if(member->loginFlag)
        {
            reply = reply + member->accountName + string("#") + member->IP_address + string("#") + 
                    to_string(member->portNum) + string("\n");
        }
    }
    return reply;
}

void AccountManager::logout(string accountName)
{
    auto target = find(this->accountInfo.begin(), this->accountInfo.end(), accountName);
    target->loginFlag = false;
}
