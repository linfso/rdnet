/*

 length  |  id    | type  |  index  |  frame  |
 ushort  | int    | byte  |  ushort |  byte   |

type 0xFO video  0xF1 audio  0x00 CMD
lengeth  < 2048
frame   0x10 I frame  0x00 pframe
        0x00 begin    0x01    0x02 end


 length  |  id     | type  | cmd
 ushort  | int     | byte  | int

 cmd: 0x0010 alive   0x0011 stop


*/

#ifndef COMMON_H
#define COMMON_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <list>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <map>
#include <set>
#include "cbufque.h"

struct Connection
{
    int      sock;
    time_t   tm;

    int  bufsize;
    int  wantsize;
    int  recvsize;
    char *buf;

    int  termid;
    CBufQue bufque;

    Connection()
    {
        bufsize = 2048;
        wantsize = 0;
        recvsize = 0;
        buf = new char[2048];
        termid = 0;
        bufque.Init(2048,256);
        sock = 0;
        tm = 0;
    }

    ~Connection()
    {
        bufsize = 0;
        wantsize = 0;
        recvsize = 0;
        delete[] buf;
        buf = NULL;
        termid = 0;
        bufque.Release();
        sock = 0;
        tm = 0;
    }
};
typedef map<int, Connection*> MAPConnection; //
typedef map<int, Connection*>::iterator MAPConnectionIt;

typedef set<int> SETSocket;  //socket
typedef set<int>::iterator SETSocketIt;

typedef map<int, SETSocket*> MAPTermClient;
typedef map<int, SETSocket*>::iterator MAPTermClientIt;



#endif // COMMON_H
