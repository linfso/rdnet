#include <iostream>
#include "common.h"
#include "netcommon.h"
#include "ctcpserver.h"

using namespace std;

int main()
{

    ctcpserver server;
    server.start();

    getchar();
    return 0;
}

