#include <thread>
#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include "testserver.h"

Message::Message() :
    length(0),
    data(NULL)
{
}

Message::Message(int length) :
    length(length)
{
    data = new char[length];
    memset(data,'\0', length);
}

Message::~Message()
{
    delete[] data;
}

void Message::procData()
{
    int sum = 0;
    std::vector<int> numVec;
    std::string str(this->data);

    for(char ch : str)
    {
        if(ch >='0' && ch <= '9')
        {
            numVec.push_back(ch - '0');
            sum += ch - '0';
        }
    }
    std::sort(numVec.begin(),numVec.end(), std::greater<int>());

    printf("Sum: %d\n", sum);
    for(int a : numVec)
    {
        printf("%d",a);
    }
    printf("\n");

    if (numVec.size() > 0)
    {
        printf("Min: %d\n", *(numVec.end() - 1));
        printf("Max: %d\n", *numVec.begin());
    }
    else
    {
        printf("Min: 0\nMax: 0\n");
    }

}

int procTCPServer()
{
    int retVal = 0;
    TCPServer tcpServer;
    retVal = tcpServer.Init();
    if(retVal != 0)
    {
        return retVal;
    }
    while (1)
    {
        Message msg;
        retVal = tcpServer.ReceiveMsg(msg);
        if(retVal != 0)
        {
            return retVal;
        }

        msg.procData();

        retVal = tcpServer.SendMsg(msg);
        if(retVal != 0)
        {
            return retVal;
        }
    }

    tcpServer.Stop();
    return 0;

}
int procUDPServer()
{
    int retVal = 0;
    UDPServer udpServer;
    retVal = udpServer.Init();
    if(retVal != 0)
    {
        return retVal;
    }
    while(1)
    {
        Message msg(512);

        retVal = udpServer.ReceiveMsg(msg);
        if(retVal != 0)
        {
            return retVal;
        }
        msg.procData();

        retVal = udpServer.SendMsg(msg);
        if(retVal != 0)
        {
            return retVal;
        }
    }

    udpServer.Stop();
    return 0;
}

int main(int argc, char *argv[])
{
    Threads[0]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)procTCPServer, NULL, 0, NULL);
    Threads[1]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)procUDPServer, NULL, 0, NULL);  //создаем нити

    WaitForMultipleObjects(2, Threads, TRUE, INFINITE);//ждем пока отработают нити

    return 0;
} 


TCPServer::TCPServer()
{

}

TCPServer::~TCPServer()
{

}

int TCPServer::Init()
{
    WSADATA wsaData;
    int retVal = 0;

    WSAStartup(MAKEWORD(2,2), &wsaData);

    servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(servSock == INVALID_SOCKET)
    {
        printf("Unable to create socket\n");
        WSACleanup();
        return SOCKET_ERROR;
    }
    SOCKADDR_IN sin;
    sin.sin_family = PF_INET;
    sin.sin_port = htons(1111);
    sin.sin_addr.s_addr = INADDR_ANY;

    retVal = bind(servSock, (LPSOCKADDR)&sin, sizeof(sin));
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to bind\n");
        WSACleanup();
        return SOCKET_ERROR;
    }

    retVal = listen(servSock, 10);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to listen\n");
        WSACleanup();
        return SOCKET_ERROR;
    }

    clientSock = accept(servSock, NULL, NULL);
    if(clientSock == INVALID_SOCKET)
    {
        printf("Unable to accept\n");
        WSACleanup();
        return SOCKET_ERROR;
    }
    return 0;
}

void TCPServer::Stop()
{
    closesocket(clientSock);
    closesocket(servSock);

    WSACleanup();
}

int TCPServer::ReceiveMsg(Message &msg)
{
    //Receive Message length
    char* len = new char[4];
    int retVal = recv(clientSock, len, 4, 0);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to recv\n");
        return SOCKET_ERROR;
    }

    std::istringstream iss(len);
    if( !(iss >> msg.length))
    {
        printf("Invalid length\n");
        return 1;
    }
    printf("Length: %d\n", msg.length);

    msg.data = new char[msg.length + 1];
    memset(msg.data, '\0', msg.length + 1);

    //Receive Data
    int pcktLen = 1024;
    for(int i = 0; i < msg.length; i += pcktLen)
    {
        if (msg.length - i < pcktLen)
            pcktLen = msg.length - i;

        printf("Packet length: %d\n", pcktLen);
        retVal = recv(clientSock, msg.data + i, pcktLen, 0);
        if(retVal == SOCKET_ERROR)
        {
            printf("Unable to recv\n");
            return SOCKET_ERROR;
        }
    }


    printf("Got the request from client\n%s\n",msg.data);
    return 0;

}

int TCPServer::SendMsg(Message& msg)
{
    printf("Sending response from server\n");

    //send length
    std::string len;
    std::ostringstream oss;
    oss << msg.length;
    len = oss.str();
    int retVal = send(clientSock, len.c_str(), 4, 0);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to send\n");
        return SOCKET_ERROR;
    }

    int pcktLen = 1024;
    for(int i = 0; i < msg.length; i+= pcktLen)
    {
        if (msg.length - i < pcktLen)
            pcktLen = msg.length - i;

        retVal = send(clientSock, msg.data + i, pcktLen, 0);

        if(retVal == SOCKET_ERROR)
        {
            printf("Unable to send\n");
            return SOCKET_ERROR;
        }
    }
    return 0;
}

UDPServer::UDPServer()
{

}

UDPServer::~UDPServer()
{

}

int UDPServer::Init()
{
    struct sockaddr_in server;
    WSADATA wsa;
    int retVal = 0;
    slen = sizeof(si_other);
    WSAStartup(MAKEWORD(2,2),&wsa);

    //Create a socket
    servSock = socket(AF_INET , SOCK_DGRAM , 0 );
    if(servSock == INVALID_SOCKET)
    {
        printf("Unable to create socket\n");
        WSACleanup();
        return SOCKET_ERROR;
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    //Bind
    retVal = bind(servSock, (struct sockaddr *)&server , sizeof(server));
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to bind\n");
        WSACleanup();
        return SOCKET_ERROR;
    }

    return 0;
}

void UDPServer::Stop()
{
    closesocket(servSock);
    WSACleanup();
}

int UDPServer::ReceiveMsg(Message &msg)
{
    //Receive Message length
    char* len = new char[4];
    int retVal = recvfrom(servSock, len, 4, 0, (struct sockaddr *) &si_other, &slen);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to recv\n");
        return SOCKET_ERROR;
    }

    std::istringstream iss(len);
    if( !(iss >> msg.length))
    {
        printf("Invalid length\n");
        return 1;
    }
    printf("Length: %d\n", msg.length);

    msg.data = new char[msg.length + 1];
    memset(msg.data, '\0', msg.length + 1);


    //Receive Data
    int pcktLen = 512;
    for(int i = 0; i < msg.length; i += pcktLen)
    {
        if (msg.length - i < pcktLen)
            pcktLen = msg.length - i;

        printf("Packet length: %d\n", pcktLen);
        retVal = recvfrom(servSock, msg.data + i, pcktLen, 0, (struct sockaddr *) &si_other, &slen);
        if (retVal == SOCKET_ERROR)
        {
            printf("Unable to recv\n");
            return SOCKET_ERROR;
        }
    }
    printf("Data: %s\n" , msg.data);

    return 0;
}

int UDPServer::SendMsg(Message &msg)
{
    //send length
    std::string len;
    std::ostringstream oss;
    oss << msg.length;
    len = oss.str();
    int retVal = sendto(servSock, len.c_str(), len.length(), 0, (struct sockaddr*) &si_other, slen);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to send\n");
        return SOCKET_ERROR;
    }

    int pcktLen = 512;
    for(int i = 0; i < msg.length; i+= pcktLen)
    {
        if (msg.length - i < pcktLen)
            pcktLen = msg.length - i;

        retVal = sendto(servSock, msg.data + i, pcktLen, 0, (struct sockaddr*) &si_other, slen);
        if (retVal == SOCKET_ERROR)
        {
            printf("Unable to send\n");
            return SOCKET_ERROR;
        }
    }
    return 0;
}
