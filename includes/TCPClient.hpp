#ifndef TCPCLIENT_HHP
#define TCPCLIENT_HHP

#include <QObject>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QTcpSocket>
#include <QDebug>
#include <QIODevice>
#include "INetwork.hpp"

class Client;

class TCPClient : public  QObject, public INetwork
{
  Q_OBJECT

public:
  TCPClient(Client *, std::string const &hostname, unsigned short port, QObject *parent = 0);
  ~TCPClient();
  bool initiateService();
  void shutDown();
  bool sendBabelPacket(Protocol::BabelPacket &);
private:
  void displayError(QAbstractSocket::SocketError socketError);
private slots:
  void readMessage();

private:
  Client *client;
  std::string hostName;
  unsigned short port;
  QTcpSocket *tcpSocket;
};

#endif // TCPCLIENT_HHP
