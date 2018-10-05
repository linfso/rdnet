#ifndef CEPOLL_H
#define CEPOLL_H

#include <sys/epoll.h>

#include <iostream>
#include <string.h>
class CEpoll
{
private:

    int m_epfd;
    int m_maxfds;
    int m_curfds;
    epoll_event *m_pevents;

public:
    CEpoll();
    ~CEpoll();

    int CreateEpoll(int maxfd);
    int DestroyEpoll();
    int EpollWait(int timeout = -1);

    int EpollMod(int fd, unsigned int state);
    int EpollAdd(int fd, unsigned int state);
    int EpollDel(int fd);
    int GetEVFd(int n) const;

    int EpollMod(int fd, unsigned int state, void* ptr);
    int EpollAdd(int fd, unsigned int state, void* ptr);
    int EpollDel(int fd, void* ptr);
    void* GetEVPtr(int n) const;

    unsigned int GetEVEvents(int n) const;
    int GetMaxFds() const;
    int GetCurFds() const;

};

#endif // CEPOLL_H
