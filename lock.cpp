#include "lock.h"

CMutex::CMutex()
{
    //pthread_mutex_destroy(&m_mutex);
    //pthread_mutexattr_destroy(&m_mutexattr);
    pthread_mutexattr_init(&m_mutexattr);
}

CMutex::~CMutex()
{
    DestroyMutex();
}

int CMutex::SetMutexType(int mutextype)
{
    return pthread_mutexattr_settype(&m_mutexattr, mutextype);
}

int CMutex::GetMutexType(int *mutextype)
{
    return pthread_mutexattr_gettype(&m_mutexattr, mutextype);
}

int CMutex::SetPShared(int pshared)
{
    return pthread_mutexattr_setpshared(&m_mutexattr, pshared);
}

int CMutex::GetPShared(int *pshared)
{
    return pthread_mutexattr_getpshared(&m_mutexattr, pshared);
}

int CMutex::CreateMutex()
{
    return pthread_mutex_init(&m_mutex, &m_mutexattr);
}

int CMutex::DestroyMutex()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_mutexattr_destroy(&m_mutexattr);
    return 0;
}

int CMutex::Lock()
{
    return pthread_mutex_lock(&m_mutex);
}

int CMutex::TryLock()
{
    return pthread_mutex_trylock(&m_mutex);
}

int CMutex::UnLock()
{
    return pthread_mutex_unlock(&m_mutex);
}

/************************************************************************/
/************************************************************************/
CSemaphore::CSemaphore()
{

}

CSemaphore::~CSemaphore()
{

}

int CSemaphore::CreateSemphore()
{
    return sem_init(&m_semaphore, 0, 0);;
}
int CSemaphore::CreateSemphore(int pshared, unsigned int value)
{
    return sem_init(&m_semaphore, pshared, value);
}

int CSemaphore::DestroySemphore()
{
    return sem_destroy(&m_semaphore);
}

int CSemaphore::Post()
{
    return sem_post(&m_semaphore);
}

int CSemaphore::Wait()
{
    return sem_wait(&m_semaphore);
}

int CSemaphore::TryWait()
{
    return sem_trywait(&m_semaphore);
}
int CSemaphore::TimedWait(const struct timespec *abs_timeout)
{
    return sem_timedwait(&m_semaphore, abs_timeout);
}
/*****************************************************************************************/
/*****************************************************************************************/

CCond::CCond()
{

}

CCond::~CCond()
{
    DestroyCond();
}

int CCond::CreateCond()
{
    if(pthread_mutex_init(&m_mutex, NULL)!=0)
    {
        return -1;
    }
    return pthread_cond_init(&m_cond, NULL);
}

int CCond::DestroyCond()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
    return 0;

}

int CCond::Signal()
{
    return pthread_cond_signal(&m_cond);
}

int CCond::BroadCast()
{
    return pthread_cond_broadcast(&m_cond);
}

int CCond::Wait()
{
    pthread_mutex_lock(&m_mutex);
    pthread_cond_wait(&m_cond, &m_mutex);
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

int CCond::TimedWait(const struct timespec *abstime)
{
    pthread_mutex_lock(&m_mutex);
    pthread_cond_timedwait(&m_cond, &m_mutex, abstime);
    pthread_mutex_unlock(&m_mutex);

    return 0;
}
/************************************************************************/
/************************************************************************/

CRWLock::CRWLock()
{

}

CRWLock::~CRWLock()
{
    DestroyRWLock();
}

int CRWLock::CreateRWLock()
{
    return pthread_rwlock_init(&m_rwlock, NULL);
}

int CRWLock::DestroyRWLock()
{
    return pthread_rwlock_destroy(&m_rwlock);
}

int CRWLock::RdLock()
{
    return pthread_rwlock_rdlock(&m_rwlock);
}

int CRWLock::WrLock()
{
    return pthread_rwlock_wrlock(&m_rwlock);
}

int CRWLock::TryRdLock()
{
    return pthread_rwlock_tryrdlock(&m_rwlock);
}

int CRWLock::TryWrLock()
{
    return pthread_rwlock_trywrlock(&m_rwlock);
}

int CRWLock::UnLock()
{
    return pthread_rwlock_unlock(&m_rwlock);
}
