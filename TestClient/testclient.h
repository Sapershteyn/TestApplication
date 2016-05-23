#ifndef TESTCLIENT_H
#define TESTCLIENT_H

class TCPClient
{
private:
    SOCKET clientSock;
public:
    int Init();
    void Stop();
    int ReceiveMsg();
    int SendMsg(std::string &str);
};

class UDPClient
{
private:
    SOCKET clientSock;
    sockaddr_in si_other;
public:
    int Init();
    void Stop();
    int ReceiveMsg();
    int SendMsg(std::string str);
};


#endif // TESTCLIENT_H
