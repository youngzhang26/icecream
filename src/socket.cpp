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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include<unistd.h>
#include "log.h"

namespace icecream {

void initTcpAddrInfo(struct addrinfo &hints) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family    = AF_UNSPEC; /* Allow IPv4 or IPv6 */
    hints.ai_socktype  = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags     = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol  = 0; /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;
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
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            fd = sfd;
            break;
        } else {
            log(WARN) << "bind failed, close fd : " << sfd;
            close(sfd);
        }
    }

    freeaddrinfo(result); /* No longer needed */
    if (fd == -1) {
        return -1;
    }

    if (listen(fd, listenBackLogs) < 0) {
        close(fd);
        return -1;
    }
    return 0;
}

int Socket::initClient(const std::string &ip, int port) {
    struct addrinfo *result, *rp;
    struct addrinfo hints;
    initTcpAddrInfo(hints);

    int s = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (s != 0) {
        log(ERROR) << "getaddrinfo: " << gai_strerror(s);
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        int cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd < 0) {
            continue;
        }
        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            fd = cfd;
            break;
        } else {
            log(WARN) << "connect failed, close fd : " << cfd;
            close(cfd);
        }
    }

    freeaddrinfo(result); /* No longer needed */
    if (fd == -1) {
        return -1;
    }
    return 0;
}

void Socket::runServer() {
    
    return;
}

void Socket::closeFd() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
    return;
}

int Socket::writeBuf(const std::string &s) {
    int ret = write(fd, s.c_str(), s.size());
    return ret;
}



} // namespace icecream