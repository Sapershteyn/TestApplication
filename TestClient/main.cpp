#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include "testclient.h"

using namespace std;

int procTCPClient()
{
    TCPClient tcpClient;

    int retVal = tcpClient.Init();
    if (retVal != 0)
    {
        return retVal;
    }

    while(1)
    {
        printf("Enter message\n");
        string inputData;
        cin>>inputData;

        printf("data length:%d\n", inputData.length());
        retVal = tcpClient.SendMsg(inputData);
        if (retVal != 0)
        {
            return retVal;
        }

        retVal = tcpClient.ReceiveMsg();
        if (retVal != 0)
        {
            return retVal;
        }
    }
    tcpClient.Stop();
    return 0;

}

int procUDPClient()
{
    UDPClient udpClient;

    int retVal = udpClient.Init();
    if (retVal != 0)
    {
        return retVal;
    }    //start communication
    while(1)
    {
        printf("Enter message: ");
        string inputData;
        cin>>inputData;

        retVal = udpClient.SendMsg(inputData);
        if (retVal != 0)
        {
            return retVal;
        }

        retVal = udpClient.ReceiveMsg();
        if (retVal != 0)
        {
            return retVal;
        }

    }

    udpClient.Stop();
    return 0;
}

int main(int argc, char *argv[])
{
    int retVal = 0;
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "tcp") == 0))
    {
        retVal = procTCPClient();
    }
    else if (argc == 2 && strcmp(argv[1], "udp") == 0)
    {
        retVal = procUDPClient();
    }
    else
    {
        printf("Invalid protocol\n");
        return 1;
    }

    return retVal;
}

TCPClient::TCPClient()
{

}

TCPClient::~TCPClient()
{

}

int TCPClient::Init()
{
    WORD ver = MAKEWORD(2,2);
    WSADATA wsaData;
    int retVal=0;

    WSAStartup(ver,(LPWSADATA)&wsaData);

    LPHOSTENT hostEnt = gethostbyname("localhost");

    if(!hostEnt)
    {
        printf("Unable to collect gethostbyname\n");
        WSACleanup();
        return 1;
    }

    //Создаем сокет
    clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(clientSock == SOCKET_ERROR)
    {
        printf("Unable to create socket\n");
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN serverInfo;

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr = *((LPIN_ADDR)*hostEnt->h_addr_list);
    serverInfo.sin_port = htons(1111);

    retVal=connect(clientSock,(LPSOCKADDR)&serverInfo, sizeof(serverInfo));
    if(retVal==SOCKET_ERROR)
    {
        printf("Unable to connect\n");
        WSACleanup();
        return 1;
    }

    printf("Connection made sucessfully\n");

    return 0;
}

void TCPClient::Stop()
{
    closesocket(clientSock);
    WSACleanup();
}

int TCPClient::ReceiveMsg()
{
    char* len = new char[4];
    int retVal = recv(clientSock, len, 4, 0);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to recv\n");
        return SOCKET_ERROR;
    }

    std::istringstream iss(len);
    int length = 0;
    if( !(iss >> length))
    {
        printf("Invalid length\n");
        return 1;
    }

    char* data = new char[length + 1];
    memset(data, '\0', length + 1);

    int pcktLen = 1024;
    for(int i = 0; i < length; i += pcktLen)
    {
        if (length - i < pcktLen)
            pcktLen = length - i;
        retVal = recv(clientSock, data + i, pcktLen, 0);

        if(retVal == SOCKET_ERROR)
        {
            printf("Unable to recv\n");
            WSACleanup();
            return 1;
        }
    }
    printf("Got the response from server\n%s\n",data);
    return 0;
}

int TCPClient::SendMsg(string &str)
{
    std::string len;
    std::ostringstream oss;
    oss << str.length();
    len = oss.str();
    int retVal = send(clientSock, len.c_str(), len.length(), 0);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to send\n");
        return SOCKET_ERROR;
    }

    int pcktLen = 1024;
    for(int i = 0; i < str.length(); i+= pcktLen)
    {
        if (str.length() - i < pcktLen)
            pcktLen = str.length() - i;
        printf("Sending request from client\n");
        retVal = send(clientSock, str.c_str()+ i, pcktLen, 0);

        if(retVal == SOCKET_ERROR)
        {
            printf("Unable to send\n");
            WSACleanup();
            return 1;
        }
    }
    return 0;
}

UDPClient::UDPClient()
{

}

UDPClient::~UDPClient()
{
}

int UDPClient::Init()
{
    WSADATA wsa;

    //Initialise winsock
    printf("\nInitialising Winsock...");
    WSAStartup(MAKEWORD(2,2),&wsa);

    LPHOSTENT hostEnt = gethostbyname("localhost");
    if(!hostEnt)
    {
        printf("Unable to collect gethostbyname\n");
        WSACleanup();
        return 1;
    }
    clientSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    //create socket
    if(clientSock == SOCKET_ERROR)
    {
        printf("Unable to create socket\n");
        WSACleanup();
        return 1;
    }

    //setup address structure
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(8888);
    si_other.sin_addr = *((LPIN_ADDR)*hostEnt->h_addr_list);

    return 0;
}

void UDPClient::Stop()
{
    closesocket(clientSock);
    WSACleanup();
}

int UDPClient::ReceiveMsg()
{
    int slen = sizeof(si_other);

    char* len = new char[4];
    int retVal = recvfrom(clientSock, len, 4, 0, (struct sockaddr *) &si_other, &slen);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to recv\n");
        return SOCKET_ERROR;
    }

    std::istringstream iss(len);
    int length = 0;
    if( !(iss >> length))
    {
        printf("Invalid length\n");
        return 1;
    }

    char* data = new char[length + 1];
    memset(data, '\0', length + 1);

    int pcktLen = 512;
    for(int i = 0; i < length; i += pcktLen)
    {
        if (length - i < pcktLen)
            pcktLen = length - i;
        retVal = recvfrom(clientSock, data + i, pcktLen, 0, (struct sockaddr *) &si_other, &slen);
        if(retVal == SOCKET_ERROR)
        {
            printf("Unable to recv\n");
            WSACleanup();
            return 1;
        }
    }
    printf("Got the response from server\n%s\n",data);

    return 0;
}

int UDPClient::SendMsg(string str)
{
    int slen = sizeof(si_other);
    std::string len;
    std::ostringstream oss;
    oss << str.length();
    len = oss.str();
    int retVal = sendto(clientSock, len.c_str(), len.length() , 0 , (struct sockaddr *) &si_other, slen);
    if(retVal == SOCKET_ERROR)
    {
        printf("Unable to send\n");
        return SOCKET_ERROR;
    }

    int pcktLen = 512;
    for(int i = 0; i < str.length(); i+= pcktLen)
    {
        if (str.length() - i < pcktLen)
            pcktLen = str.length() - i;

        printf("Sending request from client\n");
        retVal = sendto(clientSock, str.c_str() + i, pcktLen , 0 , (struct sockaddr *) &si_other, slen);
        //send the message
        if(retVal == SOCKET_ERROR)
        {
            printf("Unable to send\n");
            WSACleanup();
            return 1;
        }
    }
    return 0;
}
