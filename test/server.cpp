#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

#include "src/packet.h"
#include "src/socket.h"
#include "src/log.h"

int main() {
    icecream::initLog("../log/server.log", icecream::Logger::kLevelInfo);
    icecream::Socket s;
    icecream::Packet p;
    s.initServer(4567);
    std::function<void(const std::string &, int)> f = [&](const std::string &s, int fd) {
        // LOG(INFO) << "get req: " << s;
        std::string out;
        p.encode(s, out);
        int ret = write(fd, out.c_str(), out.size());
        if (ret < out.size()) {
            LOG(INFO) << "write failed: " << strerror(errno);
            // icecream::Logger("/home/young/icecream/test/server.cpp", 21, icecream::Logger::kLevelInfo) << "write failed: " << strerror(errno);
        };
    };
    s.reg(f);
    s.runServer(2, 10);

    return 0;
}