#include "Server.hpp"

Server::Server(int port)
{
  try
  {
    this->tcpServer = new TCPServer(this, port);
    this->taskManager = new TaskManager(this->tcpServer);
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

Server::~Server()
{
  delete this->taskManager;
  delete this->tcpServer;
}

void Server::startServer()
{
  this->tcpServer->initiateService();
}

void Server::addTask(Protocol::BabelPacket &packet, int clientId)
{
  TaskManager::Task *newTask = new TaskManager::Task;
  newTask->packet = &packet;
  newTask->clientID = clientId;
  this->threadPool.putTaskInQueue(std::bind(&TaskManager::executeTask, this->taskManager, *newTask),
                                   *newTask);
}

const TaskManager &Server::getTaskManager() const
{
  return *this->taskManager;
}