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

#ifndef ICECREAM_SRC_SOCKET_H
#define ICECREAM_SRC_SOCKET_H

#include "packet.h"
#include "worker.h"
#include <map>
#include <set>
#include <string>

namespace icecream {
class Socket {
  private:
    int listenFd = -1;
    int listenBackLogs = 10000;
    char *readBuff = nullptr;
    int readMax = 64 * 1024;
    Worker works;
    std::vector<std::map<int, Packet *>> packs;
    std::vector<std::set<int>> conns;
    std::vector<IcQueue<IcReq> *> qus;
    std::function<void(const std::string &, int)> f = nullptr;
    bool ioProcess = true;
    int queueSize = 1000;

  public:
    int initServer(int port);
    int initClient(const std::string &ip, int port);

    void runServer(int ioNum = 1, int workNum = 10);

    int writeBuf(int fd, const std::string &s);

    int readBuf(int fd, std::string &s);

    void closeFd(int fd);

    void reg(std::function<void(const std::string &, int)> &f1);

  private:
    void setNonBlocking(int fd);

    int process(int ioIdx, int fd, char *ioBuf);

    void ioRun(int ioIdx);

    void closeConn(int idx, int fd);
};

} // namespace icecream

#endif // ICECREAM_SRC_SOCKET_H