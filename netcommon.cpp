#include "netcommon.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <string.h>
#include <unistd.h>

#define MAX_NET_CARDS 16
#define MAC_ADDR_LEN  20

NetCommon::NetCommon()
{
}

bool  NetCommon::GetLocalInfo(string &name, string &sip, string &smac, string &smask)
{

    int skfd = -1, if_cnt = 0, if_idx = 0;;
    struct ifreq if_buf[MAX_NET_CARDS];
    struct ifconf ifc;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return false;
    }

    ifc.ifc_len = sizeof(if_buf);
    ifc.ifc_buf = (char *)if_buf;

    if (!ioctl(skfd, SIOCGIFCONF, (char *)&ifc))
    {
        if_cnt = (ifc.ifc_len) / (sizeof(struct ifreq));
        for (if_idx = 0; if_idx < if_cnt; ++if_idx)
        {

            if (strcmp("lo", if_buf[if_idx].ifr_name) == 0)
            {
                continue;
            }

            name =  if_buf[if_idx].ifr_name;

            if (!(ioctl(skfd, SIOCGIFFLAGS, (char *)(&if_buf[if_idx]))))
            {
                if ((if_buf[if_idx].ifr_flags & IFF_UP) &&
                        (if_buf[if_idx].ifr_flags & IFF_RUNNING))
                {
                    if (!(ioctl(skfd, SIOCGIFHWADDR, (char *)(&if_buf[if_idx]))))
                    {
                        char bufmac[64];
                        memset(bufmac,0,64 );
                        sprintf(
                                    bufmac,
                                    "%02X:%02X:%02X:%02X:%02X:%02X",
                                    (unsigned char)(if_buf[if_idx].ifr_hwaddr.sa_data[0]),
                                (unsigned char)(if_buf[if_idx].ifr_hwaddr.sa_data[1]),
                                (unsigned char)(if_buf[if_idx].ifr_hwaddr.sa_data[2]),
                                (unsigned char)(if_buf[if_idx].ifr_hwaddr.sa_data[3]),
                                (unsigned char)(if_buf[if_idx].ifr_hwaddr.sa_data[4]),
                                (unsigned char)(if_buf[if_idx].ifr_hwaddr.sa_data[5])
                                );

                        smac = bufmac;
                    }


                    if (!(ioctl(skfd, SIOCGIFADDR, (char *)(&if_buf[if_idx]))))
                    {
                        sip = inet_ntoa(((struct sockaddr_in *)(&(if_buf[if_idx].ifr_addr)))->sin_addr);

                    }

                    if (!(ioctl(skfd, SIOCGIFNETMASK, (char *)(&if_buf[if_idx]))))
                    {
                        smask = inet_ntoa(((struct sockaddr_in *)(&(if_buf[if_idx].ifr_netmask)))->sin_addr);

                    }

                }
            }
        }
    }

    close(skfd);

    return true;
}

bool NetCommon::SetSockBlock(const int &fd, bool block)
{
    if(block)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags&~O_NONBLOCK);
    }
    else
    {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags|O_NONBLOCK);
    }
    return true;
}

bool NetCommon::SetReuseAddr(const int &fd, bool reuse)
 {
     int opt = 0;
     if(reuse)
     {
         opt = 1;
     }
     else
     {
         opt = 0;
     }

     if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
     {
         return false;
     }
     return true;
 }
