#include "cbufque.h"

CBufQue::CBufQue(void)
{
}


CBufQue::~CBufQue(void)
{
}


int CBufQue::Init( int nBufNodeLen, int QueLen )
{
    pthread_mutex_init(&m_bufMux, NULL);
    pthread_mutex_init(&m_freebufMux, NULL);

    pthread_mutex_lock(&m_freebufMux);

    m_nQueLen = QueLen;
    m_nBufNodeLen = nBufNodeLen;

    int nIndex = 0;
    for ( nIndex = 0; nIndex < QueLen; nIndex++ )
    {
        BufNode * pNode = new BufNode;
        pNode->Init(nBufNodeLen);
        m_BuffQueFree.push(pNode);
    }
    pthread_mutex_unlock(&m_freebufMux);

    return 0;
}



int CBufQue::Release()
{
  //  int count = 0;
    pthread_mutex_lock(&m_bufMux);

    while ( !m_BuffQueUse.empty() )
    {
        BufNode * pNode = m_BuffQueUse.front();
        delete pNode;
        m_BuffQueUse.pop();
    //    count++;
    }
    pthread_mutex_unlock(&m_bufMux);
    pthread_mutex_lock(&m_freebufMux);
    while ( !m_BuffQueFree.empty() )
    {
        BufNode * pNode = m_BuffQueFree.front();
        delete pNode;
        m_BuffQueFree.pop();
        //count++;
    }
    pthread_mutex_unlock(&m_freebufMux);

   // cout << "delete" << count << endl;
   pthread_mutex_destroy(&m_bufMux);
   pthread_mutex_destroy(&m_freebufMux);
   return 0;
}


int CBufQue::GetFreeNodeCount()
{
    pthread_mutex_lock(&m_freebufMux);
    int nCount = 0;
    nCount = m_BuffQueFree.size();
    pthread_mutex_unlock(&m_freebufMux);
    return nCount;
}


int CBufQue::GetUsedNodeCount()
{
    pthread_mutex_lock(&m_bufMux);
    int nCount = 0;
    nCount = m_BuffQueUse.size();
    pthread_mutex_unlock(&m_bufMux);
    return nCount;
}
int CBufQue::GetQueLen()
{
    return m_nQueLen;
}

BufNode* CBufQue::AllocNode()
{
    BufNode* pNode = NULL;
    pthread_mutex_lock(&m_freebufMux);
    if( m_BuffQueFree.empty() )
        pNode = NULL;
    else
    {
        pNode = m_BuffQueFree.front();
        m_BuffQueFree.pop();
    }

    pthread_mutex_unlock(&m_freebufMux);
    return pNode;
}


int CBufQue::PushNode(BufNode* node)
{
    pthread_mutex_lock(&m_bufMux);
    m_BuffQueUse.push(node);
    pthread_mutex_unlock(&m_bufMux);



    return 0;
}

BufNode* CBufQue::FrontNode()
{
  BufNode* pNode = NULL;
  pthread_mutex_lock(&m_bufMux);
   if( m_BuffQueUse.empty() )
       pNode = NULL;
   else
   {
       pNode = m_BuffQueUse.front();
   }
   pthread_mutex_unlock(&m_bufMux);
   return pNode;

}

BufNode* CBufQue::PopNode()
{
   BufNode* pNode = NULL;
   pthread_mutex_lock(&m_bufMux);
    if( m_BuffQueUse.empty() )
        pNode = NULL;
    else
    {
        pNode = m_BuffQueUse.front();
        m_BuffQueUse.pop();
    }
    pthread_mutex_unlock(&m_bufMux);
    return pNode;
}

int CBufQue::FreeNode(BufNode* node)
{
    pthread_mutex_lock(&m_freebufMux);
    m_BuffQueFree.push(node);
    pthread_mutex_unlock(&m_freebufMux);
    return 0;
}


int CBufQue::Reset()
{
    pthread_mutex_lock(&m_bufMux);
    pthread_mutex_lock(&m_freebufMux);
    while ( !m_BuffQueUse.empty() )
    {
        BufNode * pNode = m_BuffQueUse.front();
        m_BuffQueFree.push(pNode);
        m_BuffQueUse.pop();
    }
    pthread_mutex_unlock(&m_freebufMux);
    pthread_mutex_unlock(&m_bufMux);

    return 0;
}

