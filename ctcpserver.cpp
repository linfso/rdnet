#include "ctcpserver.h"
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

void *func_terminal(void* p);
void *func_client(void* p);
void *func_mediatask(void* p);
void *func_cmdtask(void* p);

void _outprint(char *print)
{
     timeval tv;
     gettimeofday(&tv, NULL);
     cout << tv.tv_sec << " " << tv.tv_usec << print << endl;
}

void *func_terminal(void* p) //terminal recv
{
    ctcpserver *thiz = (ctcpserver*)p;
    thiz->terminalloop();
    return 0;
}

void *func_client(void* p) //client recv,send
{
    ctcpserver *thiz = (ctcpserver*)p;
    thiz->clientloop();
    return 0;
}

void *func_mediatask(void* p) //dispach
{
    ctcpserver *thiz = (ctcpserver*)p;
    thiz->mediataskloop();
    return 0;
}

void *func_cmdtask(void* p)
{
    ctcpserver *thiz = (ctcpserver*)p;
    thiz->cmdtaskloop();
    return 0;
}

ctcpserver::ctcpserver()
{

}

ctcpserver::~ctcpserver()
{

}

int ctcpserver::start()
{
    if(m_mutexTerminal.CreateMutex() != 0)
    {
        return -1;
    }
    if(m_mutexClient.CreateMutex() != 0)
    {
        return -1;
    }
    if(m_mutexTermClient.CreateMutex() != 0)
    {
        return -1;
    }

    if(m_bufQueMedia.Init(2048, 2048) != 0)
    {
        return -1;
    }
    if(m_bufQueCMD.Init(2048, 2048) != 0)
    {
        return -1;
    }
    if(m_semMediaTask.CreateSemphore() != 0)
    {
        return -1;
    }
    if(m_semCMDTask.CreateSemphore() != 0)
    {
        return -1;
    }

    if(m_epollClient.CreateEpoll(1024) != 0)
    {
        return -1;
    }
    if(m_epollTerminal.CreateEpoll(1024) != 0)
    {
        return -1;
    }
    //terminal
    m_terminalsock = createlistensocket(2000);
    if(m_terminalsock < 0 )
    {
        return -1;
    }

    Connection  *pConnTerm = new Connection;
    pConnTerm->sock = m_terminalsock;
    pConnTerm->tm = 0;

    int iRet = m_epollTerminal.EpollAdd(m_terminalsock, EPOLLIN|EPOLLET, pConnTerm );
    if(iRet != 0)
    {
        delete pConnTerm;
        pConnTerm = NULL;
        close(m_terminalsock);
        return -1;
    }
    m_mutexTerminal.Lock();
    m_mapConnTerminal.insert(make_pair<int, Connection*>(m_terminalsock,pConnTerm));
    m_mutexTerminal.UnLock();

    //client
    m_clientsock = createlistensocket(2002);
    if(m_clientsock < 0 )
    {
        return -1;
    }

    Connection *pConnClient = new Connection;
    pConnClient->sock = m_clientsock;
    pConnClient->tm = 0;

    iRet = m_epollClient.EpollAdd(m_clientsock, EPOLLIN|EPOLLET, pConnClient);
    if(iRet != 0)
    {
        delete pConnClient;
        pConnClient = NULL;
        close(m_clientsock);
        return -1;
    }

    m_mutexClient.Lock();
    m_mapConClient.insert(make_pair<int, Connection*>(m_clientsock, pConnClient));
    m_mutexClient.UnLock();

    pthread_create(&m_thTerminal,NULL, func_terminal,this);
    pthread_create(&m_thClient, NULL, func_client,this);
    pthread_create(&m_thMediaTask,NULL, func_mediatask,this);
    pthread_create(&m_thCMDTask,NULL, func_cmdtask,this);

    return 0;
}

int ctcpserver::createlistensocket(int port)
{
    int  sockListen = socket(AF_INET, SOCK_STREAM, 0 );
    if(sockListen < 0)
    {
        return -1;
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    NetCommon::SetSockBlock(sockListen, false);
    NetCommon::SetReuseAddr(sockListen, true);

    int iRet = bind(sockListen, (sockaddr*)&addr, sizeof(addr));
    if(iRet < 0)
    {
        perror("bind error");
        close(sockListen);
        return -1;
    }

    iRet = listen(sockListen,50);
    if(iRet != 0)
    {
        close(sockListen);
        return -1;
    }

    return sockListen;
}

int ctcpserver::acceptterminal(Connection *pConn)
{
    while(true)
    {
        int newsock = accept(pConn->sock,NULL, NULL);

        if(newsock < 0)
        {
            if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                break;
            }
            return -1;
        }
        if( newsock > 0)
        {
            NetCommon::SetSockBlock(newsock,false);
            NetCommon::SetReuseAddr(newsock, true);

           if(m_mapConnTerminal.size() > 500)
           {
                close(newsock);
                return 0;
           }

            Connection *newcon = new Connection;
            newcon->sock = newsock;
            newcon->wantsize = 2;
            newcon->recvsize = 0;
            time( &(newcon->tm));

            if(m_epollTerminal.EpollAdd(newsock, EPOLLIN|EPOLLET, newcon) < 0)
            {
                delete newcon;
                newcon = NULL;
                close(newsock);
                return -1;
            }
            m_mapConnTerminal.insert(make_pair<int, Connection*>(newsock,newcon));
        }
    }

    return 0;
}

int ctcpserver::recvn(Connection *pConn)
{
    int iret = 0;
    //wantsize < 2048
    while(pConn->recvsize < pConn->wantsize)
    {
        iret = recv(pConn->sock, pConn->buf+pConn->recvsize,pConn->wantsize-pConn->recvsize, 0);
        if(iret == -1)
        {
            if(errno == EINTR)
            {
                 break;
            }
            else if(errno == EWOULDBLOCK || errno == EAGAIN)
            {
                break;
            }
            else
            {
                return -1;
            }
        }
        if(iret == 0)
        {
            return -1;
        }

        pConn->recvsize += iret;
    }

    time(&(pConn->tm));

    return  pConn->recvsize;
}

int ctcpserver::recvtrminal(Connection *pConn)
{
    int iret = 0;
    while(true)
    {
        iret = recvn(pConn);
        if(iret < 0)
        {
            close(pConn->sock);
            m_epollTerminal.EpollDel(pConn->sock,pConn);
            m_mapConnTerminal.erase(pConn->sock);
            delete pConn;
            pConn = NULL;

            return -1; //error
        }
        if(pConn->recvsize != pConn->wantsize) //no data recv
        {
            break;
        }
        else
        {
            if(pConn->wantsize == 2)
            {
                pConn->wantsize = *(short*)(pConn->buf);
                if(pConn->wantsize > 2048 || pConn->wantsize < 2)
                {
                    close(pConn->sock);
                    m_epollTerminal.EpollDel(pConn->sock,pConn);
                    m_mapConnTerminal.erase(pConn->sock);
                    delete pConn;
                    pConn = NULL;

                    return -1; //something error with data
                }
            }
            else
            {
                BufNode *pnode = m_bufQueMedia.AllocNode();
                if(pnode != NULL )
                {
                    memset( pnode->pBuf,0,pnode->nMaxLen);
                    pnode->nLen = pConn->recvsize;
                    memcpy(pnode->pBuf, pConn->buf,pConn->recvsize);
                    pnode->sock = pConn->sock;
                    m_bufQueMedia.PushNode(pnode);
                    m_semMediaTask.Post();

                }
                pConn->recvsize = 0;
                pConn->wantsize = 2;
            }
        }
    }

    return 0;
}

int ctcpserver::terminalloop()
{
    int event_count = 0;
    while(true)
    {
        event_count = m_epollTerminal.EpollWait();
        if(event_count < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            return 0;
        }
        else if(event_count > 0)
        {
            m_mutexTerminal.Lock();
            for(int index=0; index < event_count; index++)
            {
                Connection *pConn  = (Connection*)(m_epollTerminal.GetEVPtr(index));
                if(pConn->sock == m_terminalsock)
                {
                    if(acceptterminal(pConn) < 0)
                    {
                        return 0;
                    }
                }
                else
                {
                    if( m_epollTerminal.GetEVEvents(index)&EPOLLIN )
                    {
                        if(recvtrminal(pConn) < 0)
                        {
                            continue;
                        }
                    }
                    else if(m_epollTerminal.GetEVEvents(index)&EPOLLOUT)
                    {

                    }
                }
            }
            m_mutexTerminal.UnLock();
        }
    }
    return 0;
}

int ctcpserver::mediataskloop()
{
    while (true)
    {
        m_semMediaTask.Wait();
        BufNode *pnode = m_bufQueMedia.PopNode();
        if(pnode == NULL)
        {
            continue;
        }
        //id-client
        int id = *(int*)(pnode->pBuf+2);

        m_mutexTermClient.Lock();
        MAPTermClientIt itTermClient = m_mapTermClient.find(id);
        if(itTermClient == m_mapTermClient.end())
        {
            SETSocket *pSetSock = new  SETSocket;
            pSetSock->clear();
            m_mapTermClient.insert(make_pair<int, SETSocket*>(id,pSetSock));
        }
        else
        {
            SETSocket *pSetSock= itTermClient->second;
            if(!pSetSock->empty())
            {
                SETSocketIt itSock;
                m_mutexClient.Lock();
                for(itSock=pSetSock->begin(); itSock != pSetSock->end(); ++ itSock)
                {
                    int sock = *itSock;
                    MAPConnectionIt itConnClient = m_mapConClient.find(sock);
                    if(itConnClient != m_mapConClient.end())
                    {
                        Connection *pConn = itConnClient->second;
                        BufNode *pnodeClient = pConn->bufque.AllocNode();
                        if(pnodeClient == NULL)
                        {
                            continue;
                        }
                        memcpy(pnodeClient->pBuf, pnode->pBuf, pnode->nLen );
                        pnodeClient->nLen = pnode->nLen;
                        pConn->bufque.PushNode(pnodeClient);
                        m_epollClient.EpollMod(pConn->sock, EPOLLIN|EPOLLOUT, pConn);
                    }
                    else
                    {
                        pSetSock->erase(itSock);
                    }
                }
                m_mutexClient.UnLock();
            }
        }
        m_mutexTermClient.UnLock();

        m_bufQueMedia.FreeNode(pnode);
    }

    return 0;
}


int ctcpserver::acceptclient(Connection *pConn)
{
    while(true)
    {
        int newsock = accept(pConn->sock,NULL, NULL);
        if(newsock < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            if(errno == EWOULDBLOCK || errno == EAGAIN)
            {
               break;
            }
            return -1;
        }
        if( newsock > 0)
        {
            NetCommon::SetSockBlock(newsock,false);
            NetCommon::SetReuseAddr(newsock, true);

            if(m_mapConnTerminal.size() > 800)
            {
                close(newsock);
                return 0;
            }

            Connection *newcon = new Connection;
            newcon->sock = newsock;
            newcon->wantsize = 2;
            newcon->recvsize = 0;
            time( &(newcon->tm));

            if(m_epollClient.EpollAdd(newsock, EPOLLIN, newcon) < 0)
            {
                delete newcon;
                newcon = NULL;
                close(newsock);
                return -1;
            }
             m_mapConClient.insert(make_pair<int, Connection*>(newsock,newcon));

        //    cout << "m_mapConClient count:" << m_mapConClient.size() << endl;
        }
    }

    return 0;
}

int ctcpserver::recvclient(Connection *pConn)
{

    int iret = 0;
    while(true)
    {
        iret = recvn(pConn);
        if(iret < 0)
        {
            close(pConn->sock);
            m_epollClient.EpollDel(pConn->sock,pConn);
            m_mapConClient.erase(pConn->sock);
            delete pConn;
            pConn = NULL;

         //   cout << "m_mapConClient count:" << m_mapConClient.size() << endl;

            return -1; //error
        }
        if(pConn->recvsize != pConn->wantsize) //no data recv
        {
            break;
        }
        else
        {
            if(pConn->wantsize == 2)
            {
                pConn->wantsize = *(short*)(pConn->buf);
                if(pConn->wantsize > 2048 || pConn->wantsize < 2)
                {
                    close(pConn->sock);
                    m_epollClient.EpollDel(pConn->sock,pConn);
                    m_mapConClient.erase(pConn->sock);
                    delete pConn;
                    pConn = NULL;

              //      cout << "m_mapConClient count:" << m_mapConClient.size() << endl;

                    return -1; //something error with data
                }
            }
            else
            {
                BufNode *pnode = m_bufQueCMD.AllocNode();
                if(pnode != NULL )
                {
                    memset( pnode->pBuf,0,pnode->nMaxLen);
                    pnode->nLen = pConn->recvsize;
                    memcpy(pnode->pBuf, pConn->buf,pConn->recvsize);
                    pnode->sock = pConn->sock;
                    m_bufQueCMD.PushNode(pnode);
                    m_semCMDTask.Post();
                }
                pConn->recvsize = 0;
                pConn->wantsize = 2;
            }
        }
    }
    return 0;
}
int ctcpserver::sendclient(Connection *pConn)
{
   //
    BufNode *pnode = pConn->bufque.FrontNode(); //
    if(pnode != NULL)
    {
      int  iret =   send(pConn->sock, pnode->pBuf,pnode->nLen,0);
      if(iret <= 0)
       {
          if(errno != EAGAIN || errno == EWOULDBLOCK || errno == EAGAIN)
          {
                 return 0;
          }
          else
          {
              close(pConn->sock);
              m_epollClient.EpollDel(pConn->sock,pConn);
              m_mapConClient.erase(pConn->sock);
              delete pConn;
              pConn = NULL;

        //      cout << "m_mapConClient count:" << m_mapConClient.size() << endl;
              return -1;
          }

       }
      else
      {
          pConn->bufque.PopNode();
          pConn->bufque.FreeNode(pnode);
      }

    }
    if(pConn->bufque.GetUsedNodeCount() == 0)
    {
        m_epollClient.EpollMod(pConn->sock, EPOLLIN, pConn);
    }

    return 0;
}

int ctcpserver::clientloop()
{
    int event_count = 0;
    while(true)
    {
        event_count = m_epollClient.EpollWait();
        if(event_count < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            return 0;
        }
        else if(event_count > 0)
        {
            m_mutexClient.Lock();
            for(int index=0; index < event_count; index++)
            {

                Connection *pConn  = (Connection*)(m_epollClient.GetEVPtr(index));
                if(pConn->sock == m_clientsock)
                {
                    if(acceptclient(pConn) < 0)
                    {
                        return 0;
                    }  
                }
                else
                {
                    if( m_epollClient.GetEVEvents(index)&EPOLLIN )
                    {
                        if(recvclient(pConn) < 0)
                        {
                            continue;
                        }
                    }
                    if(m_epollClient.GetEVEvents(index)&EPOLLOUT)
                    {
                        sendclient(pConn);
                    }

                }
            }
            m_mutexClient.UnLock();

        }
    }

    return 0;
}

int ctcpserver::cmdtaskloop()
{
    while (true)
    {
        m_semCMDTask.Wait();
        BufNode *pnode = m_bufQueCMD.PopNode();
        if(pnode == NULL)
        {
            continue;
        }
        //deal with protocol
        cmdclientprotocol(pnode);

        m_bufQueCMD.FreeNode(pnode);
    }

    return 0;
}
int ctcpserver::cmdclientprotocol(BufNode *pnode)
{
    //id-client
    int cmd = *(int*)(pnode->pBuf+7);
    switch (cmd)
    {
    case 0x0010:
    {
        cmdclientkeepalive(pnode);
        break;
    }
    case 0x0011:
    {
        cmdclientclose(pnode);
        break;
    }
    default:
    {
        break;
    }

    }
    return 0;
}

int ctcpserver::cmdclientkeepalive(BufNode *pnode)
{
    bool bTerminalOnline = true;
    int id = *(int*)(pnode->pBuf+2);

    m_mutexTermClient.Lock();
    MAPTermClientIt itTermClient = m_mapTermClient.find(id);
    if(itTermClient != m_mapTermClient.end())
    {
        SETSocket *pset = itTermClient->second;
        SETSocketIt itsock = pset->find(pnode->sock);
        if(itsock == pset->end())
        {
            pset->insert(pnode->sock);
        }
        bTerminalOnline = true;
    }
    else
    {
        bTerminalOnline = false;
    }
    m_mutexTermClient.UnLock();

    if(!bTerminalOnline)
    {
        m_epollClient.EpollDel(pnode->sock);
        m_mutexClient.Lock();
        MAPConnectionIt itc = m_mapConClient.find(pnode->sock);
        if(itc != m_mapConClient.end())
        {
            Connection *pCon = itc->second;
            delete pCon;
            m_mapConClient.erase(itc);
            close(pnode->sock);

       //     cout << "m_mapConClient count:" << m_mapConClient.size() << endl;
        }
        m_mutexClient.UnLock();
    }

    return 0;
}

int ctcpserver::cmdclientclose(BufNode *pnode)
{
    int s = pnode->sock;
    //epoll remove the sock
    m_epollClient.EpollDel(s);

    //mapclient remove the client
    m_mutexClient.Lock();
    MAPConnectionIt itCon = m_mapConClient.find(s);
    if(itCon != m_mapConClient.end())
    {
        Connection *pConn = itCon->second;
        delete pConn;
        pConn = NULL;
        m_mapConClient.erase(itCon);
    }
    m_mutexClient.UnLock();

    //mapTermClient remove
    int id = *(int*)(pnode->pBuf+6);
    m_mutexTermClient.Lock();
    MAPTermClientIt itTermClient = m_mapTermClient.find(id);
    if(itTermClient != m_mapTermClient.end())
    {
        SETSocket *pSet = itTermClient->second;
        pSet->erase(s);
    }
    m_mutexTermClient.UnLock();
    close(s);

    return 0;
}

