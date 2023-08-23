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

#ifndef ICECREAM_SRC_WORKER_H
#define ICECREAM_SRC_WORKER_H

#include <string>
#include <vector>
#include <thread>
#include <functional>
#include "lock_free_queue.h"

namespace icecream {

struct IcReq
{
    std::string req;
    int fd;

    IcReq(std::string &s, int fd) {
        req = s;
        this->fd = fd;
    }

    IcReq() {
        this->fd = -1;
    }
};

class WorkerImpl {
private:
    bool stop = false;
    std::function<void(const std::string&, int)> f = nullptr;
    IcQueue<IcReq> qu;
    std::vector<IcQueue<IcReq>*> qus;
public:
    WorkerImpl() {
        qu.init(10000);
    }
    void addReq(IcReq &q);

    void run();

    void reg(std::function<void(const std::string&, int)>& f1) {
        f = f1;
    }

    void reg(void(*f1)(const std::string&, int)) {
        f = f1;
    }

    void stopWork();

    void setQus(const std::vector<IcQueue<IcReq>*> &q) {
        qus = q;
    }
};

class Worker {
private:
    std::vector<WorkerImpl*> works;
    std::vector<std::thread*> ts;
    std::function<void(const std::string&, int)> f = nullptr;
public:
    void init(int workers);

    void addReq(IcReq &q);

    void reg(std::function<void(const std::string&, int)>& f1);

    void close();

    void setQus(const std::vector<IcQueue<IcReq>*> &q);
};

} // namespace icecream

#endif  // ICECREAM_SRC_WORKER_H