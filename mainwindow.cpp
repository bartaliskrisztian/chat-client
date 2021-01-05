#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <string.h>
#include "winsock2.h"
#include <Ws2tcpip.h>
#include "ClientSysThread.h"
#include <QLayoutItem>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")


/*
 0 - valaki csatlakozott
 1 - valalki kilépett
 2 - mindenkinek küldés
 3 - hiba
 4 - a kliensek nevei
 5 - privát küldés
 */


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    setupStartScreen();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// letörli az összes ui elemet
void MainWindow::clearLayout() {
    if(mainLayout) {
        while(mainLayout->count()) {
            QLayoutItem *item = mainLayout->takeAt(0);
            QWidget *widget = item->widget();
            if(widget) {
                delete widget;
            }
            delete item;
        }
    }
}


// induláskor inicializájuk a szükséges ui elemeket
void MainWindow::setupStartScreen() {
    mainWidget = new QWidget();
    mainWidget->setMinimumSize(500,500);
    mainWidget->setStyleSheet("background-color:#333333");

    setCentralWidget(mainWidget);
    setWindowTitle("Chat app");

    mainLayout = new QVBoxLayout();
    mainLayout->setAlignment(Qt::AlignCenter);

    errorText = new QLabel();
    errorText->setStyleSheet("color:white;"
                             "font-size: 16px;"
                             "padding: 10px;");

    clientNameEdit = new QLineEdit();
    clientNameEdit->setPlaceholderText("Type in your name");
    clientNameEdit->setMaximumSize(200, 40);
    clientNameEdit->setStyleSheet("color:white;"
                                  "font-size: 16px;"
                                  "padding: 10px;"
                                  "border-radius: 8px;"
                                  "border: 1px solid white");

    connectButton = new QPushButton("Connect");
    connectButton->setMinimumSize(100, 35);
    connectButton->setCursor(QCursor(Qt::PointingHandCursor));
    connectButton->setStyleSheet("color:white;"
                                 "background-color: darkRed;"
                                 "border: 1px solid black;"
                                 "border-radius: 8px;"
                                 "font-size: 16px;"
                                 );

    // kapcsolódás a szerverhez
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectButtonClicked()));

    mainLayout->addWidget(errorText);
    mainLayout->addWidget(clientNameEdit);
    mainLayout->addWidget(connectButton);
    mainWidget->setLayout(mainLayout);
}

// a fő képernyőt inicializáló függvény
void MainWindow::setupMainScreen() {

    // UI elemek beállítása
    QString buttonStyle = "color:white;"
                          "background-color: darkRed;"
                          "border: 1px solid black;"
                          "border-radius: 8px;"
                          "font-size: 16px;";

    topLayout = new QHBoxLayout();
    topLayout->setAlignment(Qt::AlignCenter);
    bottomLayout = new QHBoxLayout();
    bottomLayout->setAlignment(Qt::AlignCenter);

    messages = new QTextEdit();
    messages->setEnabled(false);
    messages->setStyleSheet("color: white;"
                            "font-size: 20px;"
                            "border-radius: 8px;"
                            "border: 1px solid white;");

    clientList = new QListWidget();
    clientList->setStyleSheet("border-radius: 8px;"
                              "border: 1px solid white;");
    clientList->setMaximumSize(clientList->width()/3, this->height());

    messageEdit = new QLineEdit();
    messageEdit->setPlaceholderText("Send a message...");
    messageEdit->setMinimumHeight(50);
    messageEdit->setStyleSheet("color:white;"
                               "font-size: 16px;"
                               "border-radius: 8px;"
                               "border: 1px solid white;"
                               "padding: 10px;");

    sendButton = new QPushButton("Send");
    sendButton->setCursor(QCursor(Qt::PointingHandCursor));
    sendButton->setMinimumSize(100, 35);
    sendButton->setStyleSheet(buttonStyle);

    disconnectButton = new QPushButton("Disconnect");
    disconnectButton->setMinimumSize(100, 35);
    disconnectButton->setStyleSheet(buttonStyle);
    disconnectButton->setCursor(QCursor(Qt::PointingHandCursor));

    // eseménykezelők hozzáadása a gombokhoz
    connect(sendButton, SIGNAL(clicked()), this, SLOT(sendButtonClicked()));

    connect(disconnectButton, SIGNAL(clicked()), this, SLOT(disconnect()));


    topLayout->addWidget(messages);
    topLayout->addWidget(clientList);
    bottomLayout->addWidget(messageEdit);
    bottomLayout->addWidget(sendButton);
    bottomLayout->addWidget(disconnectButton);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
}


// kapcsolódás a szerverhez
void MainWindow::connectButtonClicked() {

    // kliens socket létrehozása
    this->ClientSocket = new QTcpSocket();

    // kapcsolódás, majd elküldjük a user nevét
    this->ClientSocket->connectToHost("127.0.0.1", 13000);

    if(!ClientSocket->waitForConnected(5000)) {
        errorText->setText(ClientSocket->errorString());
    }

    QString name = clientNameEdit->text();
    if(name.isEmpty()) {
        return;
    }
    this->clientName = name;

    // csatlakozásnál a csomag típusa 0, elküldjük a kliens nevét
    std::string message = "";
    message += '0';
    message += name.toStdString();
    QByteArray packet = QByteArray::fromStdString(message);
    if(ClientSocket->state() == QAbstractSocket::ConnectedState) {
        ClientSocket->write(packet, packet.length());
    }
    connect(ClientSocket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
}

// amikor üzenetet szeretnénk küldeni
void MainWindow::sendButtonClicked() {
    QString temp = messageEdit->text();
    std::string message = temp.toStdString();
    std::string packet = "";
    packet += '2';
    packet += this->clientName.toStdString();
    packet += '^';
    packet += message;

    QByteArray data = QByteArray::fromStdString(packet);
    if(ClientSocket->state() == QAbstractSocket::ConnectedState) {
        ClientSocket->write(data, data.length());
    }

    messageEdit->setText("");
}


// szervertől küldött üzenet fogadása
void MainWindow::socketReadyRead() {
    QByteArray data = ClientSocket->readAll();
    std::string packet = data.toStdString();
    char packetType = packet[0];
    switch (packetType) {
        // amikor valaki csatlakozik
        case '0':
        {
            std::string clientName = "";
            for(std::string::size_type i = 1; i < packet.length(); ++i){
                clientName += packet[i];
            }

            if(QString::fromStdString(clientName) == this->clientName+'\0') {
                clearLayout();
                setupMainScreen();
                messages->setText(messages->toPlainText() + "\n Csatlakoztál.");
            }
            else {
                addClientItem(QString::fromStdString(clientName));
                messages->setText(messages->toPlainText() + "\n'" + QString::fromStdString(clientName) + "' csatlakozott.");
            }

            clients.push_back(QString::fromStdString(clientName));
            break;
        }
        case '1':
        {
            std::string client = "";
            for(std::string::size_type i = 1; i < packet.length(); ++i){
                client += packet[i];
            }
            removeClientItem(QString::fromStdString(client));
            messages->setText(messages->toPlainText() + "\n'" + QString::fromStdString(client) + "' kilepett.");
            std::vector<QString>::iterator position = std::find(clients.begin(), clients.end(), QString::fromStdString(client));
            if(position != clients.end()) {
                clients.erase(position);
            }
            break;
        }
        case '2':
        {
            std::string clientName = "";
            std::string message = "";
            std::string::size_type i;
            for (i = 1; i < packet.length(); ++i) {
                if (packet[i] == '^') {
                    ++i;
                    break;
                }
                clientName += packet[i];
            }

            for (std::string::size_type j = i; j < packet.length(); ++j) {
                message += packet[j];
            }
            QString name;
            if(QString::fromStdString(clientName) == this->clientName) {
                name = QString("Te");
            }
            else {
                name = QString::fromStdString(clientName);
            }
            messages->setText(messages->toPlainText() + "\n" +
                              name + ": " +
                              QString::fromStdString(message));
            break;
        }
        case '3':
        {
            std::string error = "";
            for (std::string::size_type i = 1; i < packet.length(); ++i) {
                error += packet[i];
            }
            errorText->setText(QString::fromStdString(error));
            break;
        }
        case '4':
        {
            std::string tempName = "";
            for (std::string::size_type i = 1; i < packet.length(); ++i) {
                if(packet[i] == '^') {

                    QString newClientName = QString::fromStdString(tempName);
                    clients.push_back(newClientName);

                    if(newClientName != clientName) {
                        addClientItem(newClientName);
                    }

                    tempName = "";      
                }
                else {
                    tempName += packet[i];
                }
            }
        }
    }
}

// lecsatlakozás a szerverről
void MainWindow::disconnect() {
    ClientSocket->disconnectFromHost();
    clients.clear();
    clearLayout();
    setupStartScreen();
}

void MainWindow::addClientItem(QString name) {
    QListWidgetItem* item = new QListWidgetItem(name);
    item->setForeground(Qt::white);

    QFont font = QFont();
    font.setPointSize(20);
    item->setFont(font);
    item->setSizeHint(QSize(clientList->width(), 30));

    clientList->addItem(item);
}

void MainWindow::removeClientItem(QString name) {
    qDebug() << clientList->count();
    for(int i=0; i<clientList->count(); ++i) {
        QListWidgetItem* temp = clientList->item(i);
        qDebug() << name;
        qDebug() << temp->text();
        if(temp->text() == name) {
            clientList->removeItemWidget(temp);
            return;
        }
    }
}
