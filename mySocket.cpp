#include "mySocket.h"





namespace myRedisSentinel {


    MySocket::MySocket() {
        m_client_fd = -1;
    }


    bool MySocket::connect(const string &ip, const uint16_t &port) {
        struct sockaddr_in serv_addr;
        if ((m_client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            LOG(Error, "creation error", ip, port);
            return false;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        // Convert IPv4 and IPv6 addresses from text to binary
        if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
            LOG(Error, "invalid address/ address not supported");
            return false;
        }

        this->setSocketOpt(2, 2);

        int ret = ::connect(m_client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if (ret < 0) {
            LOG(Error,  "connection failed", ret, ip, port);
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
            LOG(Warn, "set rcv timeout Failed", ret);
        }

        timeout.tv_sec = sendSec;
        if ((ret = ::setsockopt (m_client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)) {
            LOG(Warn, "set send timeout Failed", ret);
        }
    }

    bool MySocket::send(const string &message) {
        string data = message + "\n";
        if (::send(m_client_fd, data.c_str(), data.size(), 0) < 0) {
            LOG(Error, "failed to send message", message);
            return false;
        }

        LOG(Debug, "================> send ok", message);
        return true;
    }


    bool MySocket::recv(string &message) {
        char buffer[4096*10] = {0x00};


#ifdef _SOCKET_TIMEOUT_SELECT
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(m_client_fd, &readFds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000;

        while (true) {
            int ret = select(m_client_fd + 1, &readFds, NULL, NULL, &tv);
            if (-1 == ret) {
                LOG(Error, "select error");
                break;
            } else if (0 == ret) {
                LOG(Debug, "select timeout");
                break;
            } else {
                if (FD_ISSET(m_client_fd, &readFds)) {
                    char bufferTmp[4096] = {0x00};
                    memset(bufferTmp, 0, sizeof(bufferTmp));
                    int len = ::recv(m_client_fd, bufferTmp, sizeof(bufferTmp), 0);
                    if(len > 0) {
                        LOG(Debug, len, bufferTmp);
                        bufferTmp[len] = '\0';
                        memcpy(buffer + strlen(buffer), bufferTmp, strlen(bufferTmp));
                    } else if (len < 0) {
                        LOG(Debug, "connection closed", len);
                        return false;
                    } else {
                        LOG(Debug, "no more data to read", len);
                        break;
                    }
                }
            }
        }
#else
        struct pollfd fds[1];
        fds[0].fd = m_client_fd;
        fds[0].events = POLLIN;

        int timeOutMils = 200;
        while (true) {
            int ret = poll(fds, 1, timeOutMils);
            if (-1 == ret) {
                LOG(Error, "poll error");
                break;
            } else if (ret == 0) {
                LOG(Debug, "poll timeout");
                break;
            } else {
                if (fds[0].revents & POLLIN) {
                    char bufferTmp[4096] = {0x00};
                    memset(bufferTmp, 0, sizeof(bufferTmp));
                    int len = ::recv(m_client_fd, bufferTmp, sizeof(bufferTmp), 0);
                    if (len > 0) {
                        bufferTmp[len] = '\0';
                        LOG(Debug, len, bufferTmp);
                        memcpy(buffer + strlen(buffer), bufferTmp, strlen(bufferTmp));
                    } else if (len < 0) {
                        LOG(Debug, "connection closed", len);
                        return false;
                    } else {
                        LOG(Debug, "no more data to read", len);
                        break;
                    }
                }
            }
        }
#endif

        message = buffer;

        LOG(Debug, "================> recv end", message);
        return true;
    }

    void MySocket::close() {
        // closing the connected socket
        ::close(m_client_fd);
    }

    int MySocket::getClientFd() {
        return m_client_fd;
    }



}

