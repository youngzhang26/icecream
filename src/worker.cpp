#include "worker.h"
#include <unistd.h>
#include "log.h"

namespace icecream {

void Worker::init(int workers)
{
    works.resize(workers);
    for (int i = 0; i < workers; ++i) {
        works[i] = new WorkerImpl();
        works[i]->reg(f);
        std::thread* t = new std::thread([=] () {
            works[i]->run();
        });
        ts.push_back(t);
    }
}

void Worker::close()
{
    for (int i = 0; i < works.size(); ++i) {
        works[i]->stopWork();
        ts[i]->join();
        delete ts[i];
    }
    return;
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
    f = f1;
}


void Worker::setQus(const std::vector<IcQueue<IcReq>*> &q)
{
    for (int i = 0; i < works.size(); ++i) {
        works[i]->setQus(q);
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
        bool allEmpty = true;
        for (auto &ele : qus) {
            if (ele->volatile_size() == 0) {
                continue;
            }
            IcReq q;
            while (ele->pop(&q) >= 0) {
                allEmpty = false;
                // log(INFO) << "get req " << q.req << "\n";
                if (f) {
                    f(q.req, q.fd);
                } else {
                    log(INFO) << "no func reg, do nothing\n";
                }
            }
        }
        if (allEmpty) {
            usleep(500);
        }
        if (stop) {
            break;
        }
        
    }
}

void WorkerImpl::stopWork()
{
    stop = true;
}

}