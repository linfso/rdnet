#ifndef CBUFQUE_H
#define CBUFQUE_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <queue>
#include <pthread.h>

using namespace std;

struct BufNode
{
    char * pBuf;
    int    nLen;
    int    nMaxLen;
    int    sock;


    BufNode()
    {
        pBuf    = 0;
        nLen    = 0;
        nMaxLen = 0;
        sock = 0;

    }

    ~BufNode()
    {
        if( pBuf != 0 )
        {
            delete [] pBuf;
            pBuf    = 0;
            nLen    = 0;
            nMaxLen = 0;
            sock = 0;
        }
    }

    int Init(int nBufLen)
    {
        pBuf    = new char[nBufLen];
        nLen    = 0;
        nMaxLen = nBufLen;
        return 0;
    }

};

class CBufQue
{
public:
    CBufQue(void);
     ~CBufQue(void);

public:

    int Init(int nBufNodeLen, int QueLen);
    int Release();

    BufNode* AllocNode();
    int PushNode(BufNode* node);

    BufNode* FrontNode();
    BufNode* PopNode();
    int FreeNode(BufNode* node);

    int GetFreeNodeCount();
    int GetUsedNodeCount();
    int GetQueLen();

    int Reset();

private:
    int                 m_nQueLen;
    int                 m_nBufNodeLen;

    queue<BufNode*>     m_BuffQueUse;
    queue<BufNode*>     m_BuffQueFree;
    pthread_mutex_t     m_bufMux;
    pthread_mutex_t     m_freebufMux;
};

#endif // CBUFQUE_H
