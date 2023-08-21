#include "src/socket.h"
#include "src/packet.h"
#include <unistd.h>
#include <iostream>

int main() {
    icecream::Socket c;
    icecream::Packet p;
    c.initClient("127.0.0.1", 4567);
    for (int i = 0; i < 1000; ++i) {
        std::string msg = "msg " + std::to_string(i);
        std::string out;
        p.encode(msg, out);
        c.writeBuf(out);

        std::string respIn;
        c.readBuf(respIn);
        std::vector<std::string> resps;
        p.decode(respIn, resps);
        for (auto &ele : resps) {
            std::cout << "get resp: " << ele << std::endl;
        }
    }
    c.closeFd();
    
    return 0;
}