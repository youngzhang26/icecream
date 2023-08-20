#include "src/socket.h"

int main() {
    icecream::Socket c;
    c.initClient("127.0.0.1", 4567);
    for (int i = 0; i < 1000; ++i) {
        c.writeBuf("msg " + std::to_string(i));
    }
    c.closeFd();
    
    return 0;
}