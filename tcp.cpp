#include "tcp.h"
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace
{
    const int BUFFER_SIZE = 30720;

    void log(const std::string &message)
    {
        std::cout << message << std::endl;
    }

    void exitWithError(const std::string &errorMessage)
    {
        log("ERROR: " + errorMessage);
        exit(1);
    }
}

namespace http
{
    TcpServer::TcpServer(std::string ip_address, int port)
        : m_ip_address(ip_address),
          m_port(port),
          m_socket(),
          m_new_socket(),
          m_incomingMessage(),
          m_socketAddress(),
          m_socketAddress_len(sizeof(m_socketAddress))
    {
        m_socketAddress.sin_family = AF_INET;
        m_socketAddress.sin_port = htons(m_port);
        m_socketAddress.sin_addr.s_addr = inet_addr(m_ip_address.c_str());

        startServer();
    }

    int TcpServer::startServer()
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        int option = 1;
        setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (m_socket < 0)
        {
            exitWithError("cannot create socket");
            return 1;
        }

        if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) < 0)
        {
            exitWithError("Cannot connect socket to address");
            return 1;
        }

        return 0;
    }

    TcpServer::~TcpServer()
    {
        closeServer();
    }

    void TcpServer::closeServer()
    {
        close(m_socket);
        close(m_new_socket);
        exit(0);
    }

    void TcpServer::acceptConnection(int &new_socket)
    {
        new_socket = accept(m_socket, (sockaddr *)&m_socketAddress,
                            &m_socketAddress_len);

        if (new_socket < 0)
        {
            std::ostringstream ss;
            ss << "Server failed to accept incoming connection from ADDRESS: "
               << inet_ntoa(m_socketAddress.sin_addr) << "\n PORT: "
               << ntohs(m_socketAddress.sin_port);

            exitWithError(ss.str());
        }
    }

    void TcpServer::startListen()
    {
        if (listen(m_socket, 20) < 0)
        {
            exitWithError("Socket connection failed");
        }

        std::ostringstream ss;
        ss << "\n *** Listening on ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr)
           << " PORT: " << ntohs(m_socketAddress.sin_port) << " ***\n\n";
        log(ss.str());

        int bytesReceived;

        while (true)
        {
            log("=== Waiting for new connection === \n\n\n");

            acceptConnection(m_new_socket);

            char buffer[BUFFER_SIZE] = {0};

            bytesReceived = read(m_new_socket, buffer, BUFFER_SIZE);

            if (bytesReceived < 0)
            {
                exitWithError("Failed to read bytes from client socket connection");
            }

            log("\x1b[1;34m----- Received request from client! ----- \x1b[m\n\n");

            std::string request(buffer);
            log(request + "\n");

            sendResponse(request);

            close(m_new_socket);
        }
    }

    std::string TcpServer::buildResponse(std::string &request)
    {
        int bodyStart = request.find("{");
        request.erase(0, bodyStart);

        std::ostringstream htmlFile;

        htmlFile << "<!DOCTYPE html>"
                    "<html lang=\"en\">"
                    "<body style=\"font-family: sans-serif\">"
                    "<h1> Hello From Your Server!  </h1>"
                    "<p> I heard you say:<br><p style=\"color: darkblue\"> \n\n"
                 << request
                 << "\n\n</p></body>"
                    "</html>";

        std::string htmlString = htmlFile.str();

        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\nContent-Type: text/html\n Content-Length: " << htmlString.size() << "\n\n"
           << htmlString;

        return ss.str();
    }

    void TcpServer::sendResponse(std::string &request)
    {
        std::string response = buildResponse(request);

        long bytesSent;

        bytesSent = write(m_new_socket, response.c_str(), response.size());

        if (bytesSent == response.size())
        {
            log("\x1b[1;32m----- Server response sent to client! ----- \x1b[m\n\n");
            log(response + "\n\n\n");
        }
        else
        {
            log("Error sending response to client");
        }
    }
} // namespace http
