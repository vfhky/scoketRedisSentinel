#include "mySocket.h"





namespace myRedisSentinel {


    MySocket::MySocket() {
        m_client_fd = -1;
    }


    bool MySocket::connect(const string &ip, const uint16_t &port) {
        struct sockaddr_in serv_addr;
        if ((m_client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " " <<"Socket creation error" << std::endl;
            return false;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        // Convert IPv4 and IPv6 addresses from text to binary
        if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)
            <= 0) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " " <<"Invalid address/ Address not supported" << std::endl;
            return false;
        }

        this->setSocketOpt(2, 2);

        int ret = ::connect(m_client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if (ret < 0) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " " << "Connection Failed ret=[" << ret \
                    << "] ip=[" << ip << "] port=[" << port << "]" << std::endl;
            return false;
        }

        return true;
    }

    void MySocket::setSocketOpt(const int64_t &rcvSec, const int64_t &sendSec) {
        struct timeval timeout;
        timeout.tv_sec = rcvSec;
        timeout.tv_usec = 0;

        int ret = -1;
        if ((ret = ::setsockopt (m_client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) < 0) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " " << "set rcv timeout Failed ret=[" << ret << "]" << std::endl;
        }

        timeout.tv_sec = sendSec;
        if ((ret = ::setsockopt (m_client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " " << "set send timeout Failed ret=[" << ret << "]" << std::endl;
        }
    }

    bool MySocket::send(const string &message) {
        string data = message + "\n";
        if (::send(m_client_fd, data.c_str(), data.size(), 0) < 0) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " Failed to send message" << std::endl;
            return false;
        }

        //std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " send ok message=[" << message << "]" << std::endl;
        return true;
    }


    bool MySocket::recv(string &message) {
        char buffer[4096*10] = {0x00};



        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(m_client_fd, &readFds);


        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000;

        while (true) {
            int ret = select(m_client_fd + 1, &readFds, NULL, NULL, &tv);
            if (-1 == ret) {
                std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " select error" << std::endl;
                break;
            } else if (0 == ret) {
                // std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " select timeout" << std::endl;
                break;
            } else {
                if (FD_ISSET(m_client_fd, &readFds)) {
                    char bufferTmp[4096] = {0x00};
                    memset(bufferTmp, 0, sizeof(bufferTmp));
                    int len = ::recv(m_client_fd, bufferTmp, sizeof(bufferTmp), 0);
                    if(len > 0) {
                        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " len=[" << len << "] recv ok bufferTmp=[" << bufferTmp << "]" << std::endl;
                        bufferTmp[len] = '\0';
                        memcpy(buffer + strlen(buffer), bufferTmp, strlen(bufferTmp));
                    } else if (len < 0) {
                        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " len=[" << len << "] connection closed" << std::endl;
                        return false;
                    } else {
                        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " len=[" << len << "] no more data to read" << std::endl;
                        break;
                    }
                }
            }
        }

        message = buffer;

        //std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " ====> total message=[" << message << "]" << std::endl;
        return true;
    }

    int MySocket::selectTimeOut(const int &sec, const int &usec) {
        fd_set rset;
        struct timeval tv;

        FD_ZERO(&rset);
        FD_SET(m_client_fd, &rset);

        tv.tv_sec = sec;
        tv.tv_usec = usec;

        return select(m_client_fd+1, &rset, NULL, NULL, &tv);
    }

    void MySocket::close() {
        // closing the connected socket
        ::close(m_client_fd);
    }

    int MySocket::getClientFd() {
        return m_client_fd;
    }



}

