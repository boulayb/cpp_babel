#include "TCPServer.hpp"
#include "Server.hpp"

TCPServer::TCPServer(Server *babelServer, unsigned short port):
  acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
  maxUserId(0),
  server(babelServer)
{
}

TCPServer::~TCPServer()
{

}

bool TCPServer::initiateService()
{
  this->startAccept();
  return this->ioService.run();
}

void TCPServer::shutDown()
{

}

bool TCPServer::sendBabelPacket(Protocol::BabelPacket &packet, unsigned int id)
{
  for (Session *user : this->users)
  {
    if (user->getUserId() == id)
    {
      user->writeToClient(packet);
      return true;
    }
  }
  return false;
}

bool TCPServer::disconnectUser(unsigned int id)
{

  for (auto it = this->users.begin(); it != this->users.end(); ++it)
  {
    if ((*it)->getUserId() == id)
    {
      this->server->getTaskManager().updateContactStatusTask(id);
      (*it)->socket.close();
      this->users.erase(it);
      delete *it;
      return true;
    }
  }
  return false;
}


void TCPServer::handle_accept(Session *newSession, const boost::system::error_code &error)
{
  if (!error)
  {
    this->server->getTaskManager().updateContactStatusTask(this->maxUserId++);
    newSession->start(server, this->maxUserId);
    this->users.push_back(newSession);
  }
  else
  {
    delete newSession;
  }
  this->startAccept();
}

void TCPServer::startAccept()
{
  Session* newSession = new Session(this->ioService);
  this->acceptor.async_accept(newSession->getSocket(),
      boost::bind(&TCPServer::handle_accept, this, newSession,
        boost::asio::placeholders::error));
}