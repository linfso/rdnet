#ifndef CTCPSERVER_H
#define CTCPSERVER_H

#include <iostream>
#include <map>
#include <set>

#include "common.h"
#include "netcommon.h"
#include "cepoll.h"
#include "lock.h"
#include "cbufque.h"

using namespace std;




class ctcpserver
{
public:
    ctcpserver();
    ~ctcpserver();
    int start();

    friend void *func_terminal(void* p);
    friend void *func_client(void* p);
    friend void *func_mediatask(void* p);
    friend void *func_cmdtask(void* p);

private:

    int createlistensocket(int port);
    int terminalloop();
    int clientloop();
    int mediataskloop();
    int cmdtaskloop();

    int acceptterminal(Connection *pConn);
    int recvtrminal(Connection *pConn);

    int acceptclient(Connection *pConn);
    int recvclient(Connection *pConn);
    int sendclient(Connection *pConn);

    int cmdclientprotocol(BufNode *pnode);
    int cmdclientkeepalive(BufNode *pnode);
    int cmdclientclose(BufNode *pnode);
    int recvn(Connection *pConn);



private:

    int             m_terminalsock;
    int             m_clientsock;
    CEpoll          m_epollTerminal;
    CEpoll          m_epollClient;

    CMutex          m_mutexTerminal;
    CMutex          m_mutexClient;
    CMutex          m_mutexTermClient;
    CSemaphore      m_semMediaTask;
    CSemaphore      m_semCMDTask;
    CBufQue         m_bufQueMedia;
    CBufQue         m_bufQueCMD;

    MAPConnection   m_mapConnTerminal; //id-Connection
    MAPConnection   m_mapConClient;    //sock-connection
    MAPTermClient   m_mapTermClient;   //id-sock_set

    pthread_t       m_thTerminal;
    pthread_t       m_thClient;
    pthread_t       m_thMediaTask;
    pthread_t       m_thCMDTask;

};



#endif
