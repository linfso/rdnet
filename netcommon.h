#ifndef NETCOMMON_H
#define NETCOMMON_H

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>


using namespace std;
class NetCommon
{
public:
    NetCommon();
    static bool GetLocalInfo(string &name, string &ip, string &mac, string &mask);
    static bool SetSockBlock(const int &fd, bool block);
    static bool SetReuseAddr(const int &fd, bool reuse);
};

#endif // NETCOMMON_H
