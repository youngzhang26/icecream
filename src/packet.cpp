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

#include "packet.h"
#include <string.h>
#include "log.h"

namespace icecream {

int Packet::encode(const std::string &in, std::string &out) {
    int tempMagic = magic;
    out.append((char*)(&tempMagic), 4);
    int len = in.size();
    out.append((char*)(&len), 4);
    out.append(in);
    return 0;
}

int Packet::decode(std::string &in, std::vector<std::string> &out) {
    if (buffer.size() > 0) {
        in.append(buffer);
    }
    // decode multi msg
    int pos = 0;
    while (pos + 8 < in.size()) {
        int mag = *(int*)(in.c_str());
        if (mag != magic) {
            log(WARN) << "magic error " << mag << ", should be " << magic;
            return -1;
        }
        int len = *(int*)(in.c_str() + 4);
        if (pos + 8 + len > in.size()) {
            return 0;
        }
        out.push_back(std::string(in.c_str() + 8, len));
        pos += 8 + len;
    }
    buffer.append(in.c_str() + pos, in.size() - pos);
    
    return 0;
}

} // namespace icecream