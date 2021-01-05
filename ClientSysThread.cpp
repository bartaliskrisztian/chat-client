#include "ClientSysThread.h"

ClientSysThread::ClientSysThread(QTcpSocket *socket) {
    this->AcceptSocket = socket;
}

void ClientSysThread::run() {


}

