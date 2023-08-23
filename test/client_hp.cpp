#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include "src/socket.h"
#include "src/packet.h"
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/time.h>

uint64_t getTimeUs() {
    timeval t1;
    gettimeofday(&t1, nullptr);
    return t1.tv_sec*1000000 + t1.tv_usec;
}

int main(int argc, char** argv) {
    int connNum = 40;
    int oneThreadReqNum = 1000000;
    int threadNum = 1;
    if (argc >= 2) {
        oneThreadReqNum = atoi(argv[1]);
    }
    if (argc >= 3) {
        connNum = atoi(argv[2]);
    }
    if (argc >= 4) {
        threadNum = atoi(argv[3]);
    }
    std::cout << "oneThreadReqNum " << oneThreadReqNum << " connNum " << connNum
              << " threadNum " << threadNum << std::endl;

    auto f = [&] (int start, int end) {
        icecream::Socket c;
        int epollFd = epoll_create1(0);
        if (epollFd == -1) {
            log(WARN) << "epoll_create1 failed";
            return;
        }

        std::map<int, bool> conns;
        std::map<int, icecream::Packet*> packs;
        for (int i = 0; i < connNum; ++i) {
            int fd = c.initClient("127.0.0.1", 4567);
            if (fd < 0) {
                log(ERROR) << "init conn failed in " << i << "\n";
            }
            struct epoll_event ev;
            ev.events = EPOLLOUT;
            ev.data.fd = fd;
            if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
                log(ERROR) << "epoll_ctl: listen_sock add failed";
                return;
            }
            conns[fd] = false;
            packs[fd] = new icecream::Packet();
        }

        std::string str4k = "";
        for (int i = 0; i < 10*1024; ++i) {
            char c = 'a' + rand() % 26;
            str4k.append(1, c);
        }
        str4k.append(1, '_');

        int readMax = 64*1024;
        char* ioBuf = new char[readMax];

        #define MAX_EVENTS 40
        struct epoll_event events[MAX_EVENTS];
        std::vector<std::string> reqs;

        for (int i = start; i < end;) {
            uint64_t t1Us = getTimeUs();
            for (auto &ele : conns) {
                if (ele.second == false) {
                    std::string msg = str4k + std::to_string(i);
                    std::string out;
                    packs[ele.first]->encode(msg, out);
                    int ret = write(ele.first, out.c_str(), out.size());
                    ele.second = true;
                    ++i;
                    if (i == end) {
                        break;
                    }
                }
            }
            
            int nfds = epoll_wait(epollFd, events, MAX_EVENTS, 500);
            if (nfds == -1) {
                log(ERROR) << "epoll_wait failed";
                continue;
            }
            for (int n = 0; n < nfds; ++n) {
                int currFd = events[n].data.fd;
                int bufLen = read(currFd, ioBuf, readMax);
                if (bufLen <= 0) {
                    continue;
                }
                std::string input(ioBuf, bufLen);
                reqs.clear();
                packs[currFd]->decode(input, reqs);
                for (auto &ele : reqs) {
                    conns[currFd] = false;
                    //std::cout << "get conn " << currFd << " resp : " << ele << std::endl;
                }
            }
            /*static int statCnt = 0;
            statCnt++;
            if (statCnt % 10000 == 0) {
                std::cout << "time us " << getTimeUs() - t1Us << " has resp " << respCnt << "\n";
            }*/
           
        }

        while (true) {
            bool allRecv = true;
            for (auto &ele : conns) {
                if (ele.second == false) {
                    continue;
                } else {
                    allRecv = false;
                }
                int currFd = ele.first;
                int bufLen = read(currFd, ioBuf, readMax);
                if (bufLen <= 0) {
                    continue;
                }
                std::string input(ioBuf, bufLen);
                std::vector<std::string> reqs;
                packs[currFd]->decode(input, reqs);
                for (auto &ele : reqs) {
                    conns[currFd] = false;
                    //std::cout << "get conn " << currFd << " resp : " << ele << std::endl;
                }
            }
            if (allRecv) {
                break;
            }
        }
        for (auto& ele : conns) {
            close(ele.first);
            delete packs[ele.first];
        }
        return;
    };

    std::vector<std::thread*> tVec;
    time_t t1 = time(nullptr);
    for (int i = 0; i < threadNum; ++i) {
        std::thread* t = new std::thread(f, oneThreadReqNum*i, oneThreadReqNum*(i+1));
        tVec.push_back(t);
        // std::cout << "time: " << time(nullptr) << ", start thread " << i << std::endl;
    }

    for (int i = 0; i < threadNum; ++i) {
        tVec[i]->join();
    }
    
    time_t t2 = time(nullptr);
    std::cout << "from time " << t1 << " to time: " << t2 <<  ", all thread finish, exit" << std::endl;
    time_t diff = t2 == t1 ? 1 : t2 - t1;
    float avgSpeed = threadNum*oneThreadReqNum / float(diff);
    std::cout << "avg speed " << avgSpeed << std::endl;
    
    return 0;
}