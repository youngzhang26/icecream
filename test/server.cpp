#include "src/socket.h"

int main() {
    icecream::Socket s;
    s.initServer(4567);
    s.runServer();
    
    return 0;
}