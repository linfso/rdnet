#include "cepoll.h"

#include <iostream>
#include <stdio.h>
#include <errno.h>

CEpoll::CEpoll()
{
    m_epfd    = -1;
    m_maxfds  = 0;
    m_curfds  = 0;
    m_pevents = NULL;
}

CEpoll::~CEpoll()
{
    DestroyEpoll();
}

int CEpoll::CreateEpoll(int maxfd)
{
    m_maxfds = maxfd;
    m_epfd = epoll_create(maxfd);
    if (-1 == m_epfd)
    {
        return -1;
    }

    m_pevents = new epoll_event[maxfd];
    if (NULL == m_pevents)
    {
        return -1;
    }

    return 0;
}
int CEpoll::DestroyEpoll()
{
    m_curfds = 0;
    m_maxfds = 0;

    if (m_epfd != -1)
    {
       close(m_epfd);
       m_epfd = -1;
    }

    if (m_pevents != NULL)
    {
        delete[] m_pevents;
        m_pevents = NULL;
    }

    return 0;
}

int CEpoll::EpollWait(int timeout)
{

    int evnum = epoll_wait(m_epfd, m_pevents, m_curfds, timeout);

    if (-1 == evnum)
    {
       // printf("errno:%d", errno);
        return -1;
    }

    return evnum;
}

int CEpoll::EpollMod(int fd, unsigned int state)
{
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.events = state;
    ev.data.fd = fd;

    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev) < 0)
    {
        return -1;
    }

    return 0;
}

int CEpoll::EpollAdd(int fd, unsigned int state)
{

    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.events = state;
    ev.data.fd = fd;

    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        return -1;
    }
    m_curfds++;

    return 0;
}

int CEpoll::EpollDel(int fd)
{
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.fd = fd;
    ev.data.ptr = NULL;

    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &ev) < 0)
    {
        return -1;
    }

    m_curfds--;

    return 0;
}

int CEpoll::GetEVFd(int n) const
{
    return m_pevents[n].data.fd;
}

unsigned int CEpoll::GetEVEvents(int n) const
{
    return m_pevents[n].events;
}

int CEpoll::GetMaxFds() const
{
    return m_maxfds;
}

int CEpoll::GetCurFds() const
{
    return m_curfds;
}

int CEpoll::EpollMod(int fd, unsigned int state, void* ptr)
{
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.events = state;
    ev.data.ptr= ptr;

    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev) < 0)
    {
        return -1;
    }

    return 0;
}

int CEpoll::EpollAdd(int fd, unsigned int state, void* ptr)
{
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.events = state;
    ev.data.ptr= ptr;

    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
    {
        return -1;
    }
    m_curfds++;

    return 0;
}

int CEpoll::EpollDel(int fd, void* ptr)
{
    epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.ptr = ptr;

    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &ev) < 0)
    {
        return -1;
    }

    m_curfds--;

    return 0;
}

void* CEpoll::GetEVPtr(int n) const
{
    return m_pevents[n].data.ptr;
}
