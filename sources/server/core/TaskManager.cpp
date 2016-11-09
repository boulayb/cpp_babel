#include <iostream>
#include "TaskManager.hpp"

TaskManager::TaskManager(TCPServer *server) : network(server)
{
}

TaskManager::~TaskManager()
{

}

void TaskManager::executeTask(Task const &task)
{
  std::cout << "Trying to execute task " << (int)task.packet->code <<  std::endl;
  std::cout << task.packet->magicNbr << std::endl;
  if (task.packet->magicNbr == MAGIC_NUMBER)
  {
    auto it = this->actions.find(task.packet->code);

    if (it != this->actions.end())
    {
      std::cout << "Executing task: " << (int)task.packet->code << "with data: " << task.packet->data << std::endl;
      (this->*(it->second))(task);
      return;
    }
  }
}

void TaskManager::handShakeTask(Task const &task)
{
  Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(Protocol::BabelPacket::Code::HAND_SHAKE, nullptr, 0);
  this->network->sendBabelPacket(*packet, task.clientID);
}

void TaskManager::signInTask(Task const &task)
{
  std::vector<std::string> dataSplited = this->splitDataByDelimiter(':', task.packet->data, task.packet->dataLength);
  Protocol::BabelPacket::Code returnCode = this->database.login(dataSplited[LOGIN_INDEX], dataSplited[PASSWORD_INDEX]);
  if (returnCode == Protocol::BabelPacket::Code::SIGN_IN_SUCCESS)
  {
    this->database.setId(dataSplited[LOGIN_INDEX], task.clientID);
  }
  Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(returnCode, nullptr, 0);
  this->network->sendBabelPacket(*packet, task.clientID);
  std::string login = this->database.getLoginById(task.clientID);
  this->updateContactStatusTask(login, ONLINE_STATUS);
}

void TaskManager::signUpTask(Task const &task)
{
  std::vector<std::string> dataSplited = this->splitDataByDelimiter(':', task.packet->data, task.packet->dataLength);
  std::cout << dataSplited[LOGIN_INDEX] << " " << std::endl;
  std::cout << dataSplited[PASSWORD_INDEX] << std::endl;
  Protocol::BabelPacket::Code returnCode = this->database.registerUser(dataSplited[LOGIN_INDEX], dataSplited[PASSWORD_INDEX]);
  Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(returnCode, nullptr, 0);
  this->network->sendBabelPacket(*packet, task.clientID);
}

void TaskManager::signOutTask(Task const &task)
{
  std::string login = this->database.getLoginById(task.clientID);
  this->database.setId(login, -1);
  Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(Protocol::BabelPacket::Code::SIGN_OUT_SUCCESS, nullptr, 0);
  this->network->sendBabelPacket(*packet, task.clientID);
  this->updateContactStatusTask(login, OFFLINE_STATUS);
//  this->network->disconnectUser(task.clientID);
}

void TaskManager::getContactTask(Task const &task)
{
  std::vector<std::string> dataSplited = this->splitDataByDelimiter(':', task.packet->data, task.packet->dataLength);
  std::string user = this->database.getLoginById(task.clientID);
  const std::vector<std::string> &friendList = this->database.getFriendsList(user);
  std::string data;
  for (std::string friendStr : friendList)
  {
    data += friendStr;
    data += SEPARATOR;
    if (this->database.getId(friendStr) == -1)
      data += OFFLINE_STATUS;
    else
      data += ONLINE_STATUS;
    data += ENDSEPARATOR;
  }
  unsigned char *basicData = Protocol::Protocol::stringToPointer(data);
  Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(Protocol::BabelPacket::Code::CONTACT_LIST_SUCCESS, basicData, data.size());
  this->network->sendBabelPacket(*packet, task.clientID);
}

void TaskManager::callTask(Task const &task)
{
  std::vector<std::string> dataSplited = this->splitDataByDelimiter(':', task.packet->data, task.packet->dataLength);
  int friendId;
  if ((friendId = this->database.getId(dataSplited[LOGIN_INDEX])) != -1)
  {
    std::string user = this->database.getLoginById(task.clientID);
    std::string fullData = user + SEPARATOR + dataSplited[IP_INDEX] + SEPARATOR + dataSplited[PORT_INDEX];
    unsigned char *basicData = Protocol::Protocol::stringToPointer(fullData);
    Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(Protocol::BabelPacket::Code::CALL, basicData, fullData.size());
    this->network->sendBabelPacket(*packet, friendId);
  }
  else
  {
    Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(Protocol::BabelPacket::Code::USER_NOT_FRIEND,
                                                                     nullptr, 0);
    this->network->sendBabelPacket(*packet, task.clientID);
  }
}

void TaskManager::delContactTask(Task const &task)
{
  std::vector<std::string> dataSplited = this->splitDataByDelimiter(':', task.packet->data, task.packet->dataLength);
  std::string currentUser = this->database.getLoginById(task.clientID);
  Protocol::BabelPacket::Code errorCode = this->database.deleteFriend(currentUser, dataSplited[LOGIN_INDEX]);
  Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(errorCode, nullptr, 0);
  this->network->sendBabelPacket(*packet, task.clientID);
}

void TaskManager::addContactTask(Task const &task)
{
  std::vector<std::string> dataSplited = this->splitDataByDelimiter(':', task.packet->data, task.packet->dataLength);
  std::string currentUser = this->database.getLoginById(task.clientID);
  std::cout << "dataSplited: " << dataSplited[LOGIN_INDEX] << std::endl;
  std::cout << "pack data: " << task.packet->data << std::endl;
  Protocol::BabelPacket::Code errorCode = this->database.addFriend(currentUser, dataSplited[LOGIN_INDEX]);
  Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(errorCode, nullptr, 0);
  this->network->sendBabelPacket(*packet, task.clientID);
}

void TaskManager::connectionLostTask(Task const &task)
{
  std::string currentUser;
  try
  {
    currentUser =  this->database.getLoginById(task.clientID);
  }
  catch (std::exception)
  {
    return;
  }
  updateContactStatusTask(currentUser, OFFLINE_STATUS);
  this->database.setId(currentUser, -1);
}

void TaskManager::updateContactStatusTask(std::string const &login, std::string const &status) const
{
  const std::vector<std::string> &friendList = this->database.getFriendsList(login);
  int friendId = 0;
  for (std::string friendStr : friendList)
  {
    if ((friendId = this->database.getId(friendStr)) != -1)
    {
      std::string msg = login + SEPARATOR + status;
      unsigned char *basicData = Protocol::Protocol::stringToPointer(msg);
      Protocol::BabelPacket *packet = Protocol::Protocol::createPacket(Protocol::BabelPacket::
                                                                       Code::UPDATE_CONTACT_STATUS, basicData, msg.size());
      this->network->sendBabelPacket(*packet, static_cast<unsigned  int>(friendId));
    }
  }
}

std::vector<std::string> &TaskManager::splitDataByDelimiter(char delimiter, unsigned char *data, int size)
{
  std::vector<std::string> *dataSplited = new std::vector<std::string>;
  int nbrWords = 0;
  std::string word = "";
  for (int i = 0; i < size ; ++i)
  {
    if (data[i] == delimiter)
      {
        nbrWords++;
        dataSplited->push_back(word);
        word = "";
      }
    else
      word += data[i];
  }
  dataSplited->push_back(word);
  return *dataSplited;
}
