#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>
#include <semaphore.h>

class CMutex
{
private:
    pthread_mutex_t     m_mutex;
    pthread_mutexattr_t m_mutexattr;

public:

    CMutex();
    ~CMutex();

    int SetMutexType(int mutextype);
    int GetMutexType(int *mutextype);

    int SetPShared(int pshared);
    int GetPShared(int *pshared);

    int CreateMutex();
    int DestroyMutex();

    int Lock();
    int TryLock();
    int UnLock();

};
/***************************************************************************/
class CSemaphore
{
private:
    sem_t m_semaphore;

public:
    CSemaphore();
    ~CSemaphore();

    int CreateSemphore();
    int CreateSemphore(int pshared, unsigned int value);
    int DestroySemphore();

    int Post();
    int Wait();
    int TryWait();
    int TimedWait(const struct timespec *abs_timeout);

};
/************************************************************************************/
class CCond
{
private:
    pthread_cond_t     m_cond;
    pthread_mutex_t    m_mutex;

public:
    CCond();
    ~CCond();

    int CreateCond();
    int DestroyCond();

    int Signal();
    int BroadCast();

    int Wait();
    int TimedWait(const struct timespec *abstime);
};
/***********************************************************************************/
class CRWLock
{
private:
    pthread_rwlock_t m_rwlock;

public:
    CRWLock();
    ~CRWLock();

    int CreateRWLock();
    int DestroyRWLock();
    int RdLock();
    int WrLock();
    int TryRdLock();
    int TryWrLock();
    int UnLock();

};






#endif // LOCK_H
