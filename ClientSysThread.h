#ifndef CLIENTSYSTHREAD_H
#define CLIENTSYSTHREAD_H

#pragma once
#include <stdio.h>
#include "winsock2.h"
#include <Ws2tcpip.h>
#include <QTcpSocket>

#pragma comment(lib, "ws2_32.lib")

#include "SysThread.h"

class ClientSysThread : public SysThread
{
private:
    QTcpSocket *AcceptSocket;
public:
    ClientSysThread(QTcpSocket*);
    virtual void run();
};

#endif // CLIENTSYSTHREAD_H
