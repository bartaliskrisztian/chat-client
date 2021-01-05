#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QListWidget>
#include <vector>
#include <QMap>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // attributes
    Ui::MainWindow *ui;
    QTcpSocket *ClientSocket;
    QString clientName;
    QWidget* mainWidget;
    std::vector<QString> clients;
    QMap<QString, QWidget*> privateChat;


    QVBoxLayout* mainLayout;
    QHBoxLayout* topLayout;
    QHBoxLayout* bottomLayout;

    // attributes for welcome screen
    QLineEdit* clientNameEdit;
    QPushButton* connectButton;
    QLabel* errorText;

    // attributes for main screen
    QTextEdit* messages;
    QLineEdit* messageEdit;
    QPushButton* sendButton;
    QPushButton* disconnectButton;
    QListWidget* clientList;

    // functions
    void clearLayout();
    void setupStartScreen();
    void setupMainScreen();
    void addClientItem(QString);
    void removeClientItem(QString);
    void setupPrivateWindow(QWidget*);

private slots:

    void connectButtonClicked();
    void sendButtonClicked();
    void socketReadyRead();
    void disconnect();
    void onClientClicked(QListWidgetItem*);

};
#endif // MAINWINDOW_H
