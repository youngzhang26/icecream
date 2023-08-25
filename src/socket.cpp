// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "socket.h"

#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <set>
#include <vector>

#include "log.h"

namespace icecream {

void initTcpAddrInfo(struct addrinfo &hints) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
}

int Socket::initServer(int port) {
    struct addrinfo *result, *rp;
    struct addrinfo hints;
    initTcpAddrInfo(hints);

    int s = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &result);
    if (s != 0) {
        log(ERROR) << "getaddrinfo: " << gai_strerror(s);
        return -1;
    }

    int optval = 1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        int sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd < 0) {
            continue;
        }
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            listenFd = sfd;
            break;
        } else {
            log(WARN) << "bind failed, close fd : " << sfd;
            close(sfd);
        }
    }

    freeaddrinfo(result); /* No longer needed */
    if (listenFd == -1) {
        return -1;
    }

    if (listen(listenFd, listenBackLogs) < 0) {
        close(listenFd);
        return -1;
    }

    return 0;
}

int Socket::initClient(const std::string &ip, int port) {
    readBuff = new char[readMax];
    if (readBuff == nullptr) {
        log(ERROR) << "malloc readBuff failed ";
        return -1;
    }

    struct addrinfo *result, *rp;
    struct addrinfo hints;
    initTcpAddrInfo(hints);

    int s = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (s != 0) {
        log(ERROR) << "getaddrinfo: " << gai_strerror(s);
        return -1;
    }
    int fd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        int cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd < 0) {
            continue;
        }
        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            fd = cfd;
            break;
        } else {
            log(ERROR) << "connect failed, close fd : " << cfd;
            close(cfd);
        }
    }

    freeaddrinfo(result); /* No longer needed */
    if (fd == -1) {
        return -1;
    }
    return fd;
}

void Socket::runServer(int ioNum, int workNum) {
    log(INFO) << "begin runServer";
    works.init(workNum);
    if (workNum > 0) {
        ioProcess = false;
    }
    std::vector<std::thread *> ts;
    qus.resize(ioNum);
    packs.resize(ioNum);
    conns.resize(ioNum);
    for (int i = 0; i < ioNum; ++i) {
        qus[i] = new IcQueue<IcReq>();
        qus[i]->init(queueSize);
        std::thread *t = new std::thread([=]() { ioRun(i); });
        ts.push_back(t);
    }
    works.setQus(qus);
    for (int i = 0; i < ioNum; ++i) {
        ts[i]->join();
        delete ts[i];
    }
    works.close();
    close(listenFd);
    return;
}

void Socket::ioRun(int ioIdx) {
    // add listen fd to epoll
    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        log(WARN) << "epoll_create1 failed";
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenFd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &ev) == -1) {
        log(ERROR) << "epoll_ctl: listen_sock add failed";
        return;
    }

    char *ioBuff = new char[readMax];
    if (ioBuff == nullptr) {
        log(ERROR) << "malloc ioBuff failed ";
        return;
    }

#define MAX_EVENTS 40
    struct epoll_event events[MAX_EVENTS];
    while (true) {
        int nfds = epoll_wait(epollFd, events, MAX_EVENTS, 500);
        if (nfds == -1) {
            log(ERROR) << "epoll_wait failed";
            continue;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listenFd) {
                struct sockaddr addr;
                socklen_t addrlen = 0;
                int conn_sock = accept(listenFd, (struct sockaddr *)&addr, &addrlen);
                if (conn_sock == -1) {
                    log(ERROR) << "accept failed";
                    continue;
                }
                log(DEBUG) << "accept conn " << conn_sock;
                packs[ioIdx][conn_sock] = new Packet();
                if (packs[ioIdx][conn_sock] == nullptr) {
                    log(ERROR) << "new packet failed, re alloc";
                    packs[ioIdx][conn_sock] = new Packet();
                }
                conns[ioIdx].insert(conn_sock);
                setNonBlocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                    log(ERROR) << "epoll_ctl: conn_sock add failed";
                    continue;
                }
            } else {
                if (events[n].events == EPOLLRDHUP) {
                    closeConn(ioIdx, events[n].data.fd);
                } else {
                    int ret = process(ioIdx, events[n].data.fd, ioBuff);
                    if (ret < 0) {
                        closeConn(ioIdx, events[n].data.fd);
                    }
                }
            }
        }
    }
    log(INFO) << "io run finished.";
    for (auto ele : conns[ioIdx]) {
        close(ele);
    }
    conns[ioIdx].clear();
    close(epollFd);

    return;
}

void Socket::closeConn(int idx, int fd) {
    log(DEBUG) << "closeConn: close fd " << fd;
    if (packs[idx].count(fd) == 1) {
        delete packs[idx][fd];
        packs[idx].erase(fd);
    }
    if (conns[idx].count(fd) == 1) {
        conns[idx].erase(fd);
    }
    close(fd);
}

void Socket::closeFd(int fd) {
    log(DEBUG) << "closeFd: close fd " << fd;
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
    return;
}

int Socket::writeBuf(int fd, const std::string &s) {
    int ret = write(fd, s.c_str(), s.size());
    return ret;
}

int Socket::readBuf(int fd, std::string &s) {
    char *temp = readBuff;
    int totalLen = 0;
    while (true) {
        int ret = read(fd, temp, readMax);
        if (ret < readMax) {
            totalLen += ret;
            s = std::string(readBuff, totalLen);
            break;
        } else {
            temp += readMax;
            totalLen += readMax;
        }
    }

    return 0;
}

void Socket::setNonBlocking(int fd) {
    int oldOpt = fcntl(fd, F_GETFL);
    int newOpt = oldOpt | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOpt);
    return;
}

int Socket::process(int ioIdx, int fd, char *ioBuf) {
    while (true) {
        // std::cout << "process once" << std::endl;
        int bufLen = read(fd, ioBuf, readMax);
        if (bufLen == 0) {
            return -1;
        } else if (bufLen < 0) {
            if (errno == EAGAIN) {
                return 0;
            } else {
                log(INFO) << "read failed " << strerror(errno);
                return -1;
            }
        }
        std::string input(ioBuf, bufLen);
        std::vector<std::string> reqs;
        packs[ioIdx][fd]->decode(input, reqs);
        for (auto &ele : reqs) {
            // log(INFO) << "add req: " << ele << " to worker" << std::endl;
            if (ioProcess) {
                f(ele, fd);
            } else {
                IcReq req(ele, fd);
                qus[ioIdx]->push(req);
            }
        }
    }

    return 0;
}

void Socket::reg(std::function<void(const std::string &, int)> &f1) {
    log(DEBUG) << "Socket set server Handle.";
    f = f1;
    works.reg(f1);
    return;
}

} // namespace icecream