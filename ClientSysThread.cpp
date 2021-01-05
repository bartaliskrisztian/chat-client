#include "ClientSysThread.h"

ClientSysThread::ClientSysThread(QTcpSocket *socket, QString client, QString friendName) {
    this->AcceptSocket = socket;
    this->clientName = client;
    this->friendName = friendName;
    ui = new QWindow();
}

void ClientSysThread::run() {
    ui->setTitle("Private chat with " + friendName);
    ui->show();
}

