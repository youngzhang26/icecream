#include <unistd.h>
#include <iostream>
#include <vector>
#include "src/socket.h"
#include "src/packet.h"

int main(int argc, char** argv) {
    int threadNum = 100;
    int oneThreadReqNum = 100000;
    if (argc == 2) {
        oneThreadReqNum = atoi(argv[1]);
    }
    std::atomic<int> cnt;
    auto send = [&] (int start, int end) {
        icecream::Socket c;
        icecream::Packet p;
        c.initClient("127.0.0.1", 4567);
        std::string str4k = "";
        for (int i = 0; i < 4*1024; ++i) {
            char c = 'a' + rand() % 26;
            str4k.append(1, c);
        }
        for (int i = start; i < end; ++i) {
            std::string msg = str4k + std::to_string(i);
            std::string out;
            p.encode(msg, out);
            c.writeBuf(out);

            std::string respIn;
            c.readBuf(respIn);
            std::vector<std::string> resps;
            p.decode(respIn, resps);
            for (auto &ele : resps) {
                if (ele.size() > 4096) {
                    cnt++;
                } else {
                    std::cout << "get resp: " << ele << std::endl;
                }
            }
        }
        c.closeFd();
    };

    std::vector<std::thread*> tVec;
    time_t t1 = time(nullptr);
    for (int i = 0; i < threadNum; ++i) {
        std::thread* t = new std::thread(send, oneThreadReqNum*i, oneThreadReqNum*(i+1));
        tVec.push_back(t);
        std::cout << "time: " << time(nullptr) << ", start thread " << i << std::endl;
    }

    for (int i = 0; i < threadNum; ++i) {
        tVec[i]->join();
    }
    time_t t2 = time(nullptr);
    std::cout << "time: " << t2 <<  ", all thread finish, exit" << std::endl;
    time_t diff = t2 == t1 ? 1 : t2 - t1;
    float avgSpeed = cnt.load() / float(diff);
    std::cout << "avg speed " << avgSpeed << std::endl;
    
    return 0;
}