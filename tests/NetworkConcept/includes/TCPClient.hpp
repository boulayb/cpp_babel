#ifndef TCPCLIENT_HHP
#define TCPCLIENT_HHP

#include "INetwork.hpp"
#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDebug>
#include <QIODevice>


class TCPClient : public  QObject, public INetwork
{
  Q_OBJECT

public:
  TCPClient(std::string const &hostname, unsigned short port, QObject *parent = 0);
  ~TCPClient();
  bool initiateService();
  void shutDown();
  bool sendBabelPacket(Protocol::BabelPacket &);
private slots:
  void readMessage();
private:
  std::string hostName;
  unsigned short port;
  QTcpSocket *tcpSocket;
};

#endif // TCPCLIENT_HHP