#include "worker.h"
#include <unistd.h>
#include "log.h"

namespace icecream {

void Worker::init(int workers)
{
    works.resize(workers);
    for (int i = 0; i < workers; ++i) {
        works[i] = new WorkerImpl();
        std::thread* t = new std::thread([=] () {
            works[i]->run();
        });
        ts.push_back(t);
    }
}

void Worker::addReq(IcReq &q)
{
    static int last = 0;
    last++;
    int idx = last % works.size();
    works[idx]->addReq(q);
}

void Worker::reg(std::function<void(const std::string&, int)> &f1)
{
    for (int i = 0; i < works.size(); ++i) {
        works[i]->reg(f1);
    }
}

void WorkerImpl::addReq(IcReq &q)
{
    qu.push(q);
}

void WorkerImpl::run()
{
    // log(INFO) << "begin run \n";
    while (true) {
        if (qu.volatile_size() == 0) {
            usleep(100);
        } else {
            IcReq q;
            while (qu.pop(&q) >= 0) {
                if (f) {
                    f(q.req, q.fd);
                } else {
                    log(INFO) << "no func reg, do nothing\n";
                }
            }
        }
        if (stop) {
            break;
        }
        
    }
}

}