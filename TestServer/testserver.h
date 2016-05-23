#ifndef TESTSERVER_H
#define TESTSERVER_H

HANDLE Threads[2];

class Message
{
public:
    Message();
    Message(int length);
    ~Message();

    int length;
    char* data;

    void procData();

};

class TCPServer
{
private:
    SOCKET servSock;
    SOCKET clientSock;
public:
    int Init();
    void Stop();
    int ReceiveMsg(Message &msg);
    int SendMsg(Message &msg);
};

class UDPServer
{
private:
    SOCKET servSock;
    int slen;
    struct sockaddr_in si_other;
public:

    int Init();
    void Stop();
    int ReceiveMsg(Message &msg);
    int SendMsg(Message &msg);
};


#endif // TESTSERVER_H

