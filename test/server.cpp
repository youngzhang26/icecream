#include "src/packet.h"
#include "src/socket.h"
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

int main() {
    icecream::Socket s;
    icecream::Packet p;
    s.initServer(4567);
    std::function<void(const std::string &, int)> f = [&](const std::string &s, int fd) {
        // std::cout << "get req: " << s << std::endl;
        std::string out;
        p.encode(s, out);
        int ret = write(fd, out.c_str(), out.size());
        if (ret < out.size()) {
            std::cout << "write failed: " << strerror(errno) << std::endl;
        };
    };
    s.reg(f);
    s.runServer(2, 10);

    return 0;
}