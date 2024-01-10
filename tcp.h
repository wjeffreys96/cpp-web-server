#ifndef INCLUDED_HTTP_TCPSERVER_LINUX
#define INCLUDED_HTTP_TCPSERVER_LINUX

#include <sys/socket.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

namespace http
{
    class TcpServer
    {
    public:
        TcpServer(std::string ip_address, int port);
        ~TcpServer();
        void startListen();

    private:
        std::string m_ip_address;
        int m_port;
        int m_socket;
        int m_new_socket;
        long m_incomingMessage;
        struct sockaddr_in m_socketAddress;
        unsigned int m_socketAddress_len;
        std::string response;

        int startServer();
        void closeServer();
        void acceptConnection(int &new_socket);
        std::string buildResponse(std::string& params);
        void sendResponse(std::string& params);
    };

} // namespace http

#endif