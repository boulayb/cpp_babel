#ifndef GUI_HH
#define GUI_HH

#include <QApplication>
#include "mainwindow.h"
#include "client.hpp"

class client;

class Gui
{
public:
    Gui(int ac, char **av);
    ~Gui() {};

    void    start();
    void    setClient(Client *client);
    void    AddContact(const std::string &contactName);
    void    askRegister(const std::string &username, const std::string &password);
    void    askLogin(const std::string &username, const std::string &password);
    void    askLogout();
    void    call(const std::string &contactName);
    void    removeContact(const std::string &contacName);
    void    newError(const std::string &error);

    void    UpdateContactList(std::vector<std::pair<std::string, bool>>);
    void    UpdateContact(const std::pair<std::string, bool> &contact);
    void    setLoginView();
    void    Login();
    void    affInfoMessage(std::string const &);
    void    incommingCall(const std::string &userName);
    void    endCall();
    void    answerCall();



private:
    MainWindow  *mainWindow;
    Client      *client;
    QApplication app;
};

#endif // GUI_HH
