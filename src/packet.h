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

#ifndef ICECREAM_SRC_PACKET_H
#define ICECREAM_SRC_PACKET_H

#include <string>
#include <vector>

namespace icecream {

class Packet {
private:
    static const int magic = 0x49434500;
    std::string buffer;
    std::string temp;
public:
    Packet() {
        temp.reserve(64*1024);
    }
    int encode(const std::string &in, std::string &out);

    int decode(std::string &in, std::vector<std::string> &out);
};
} // namespace icecream

#endif  // ICECREAM_SRC_PACKET_H