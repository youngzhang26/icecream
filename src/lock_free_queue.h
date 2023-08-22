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


#ifndef ICECREAM_SRC_LOCK_FREE_QUEUE_H
#define ICECREAM_SRC_LOCK_FREE_QUEUE_H

#include <atomic>
#include "log.h"

namespace icecream {

template <typename T>
class IcQueue {
public:
    IcQueue() : upPos(1), 
                lowPos(1), 
                capacity(0), 
                buffer(nullptr) {
    }
    
    ~IcQueue() {
        delete [] buffer;
        buffer = nullptr;
    }

    int init(int cap) {
        if (capacity != 0) {
            log(ERROR) << "queue inited.\n";
            return -1;
        }
        if (cap == 0) {
            log(ERROR) << "queue cap should not be zero.\n";
            return -1;
        }
        buffer = new(std::nothrow) T[cap];
        if (buffer == nullptr) {
            log(ERROR) << "alloc buffer failed.\n";
            return -1;
        }
        capacity = cap;
        return 0;
    }

    int push(T& x) {
        const size_t up = upPos.load(std::memory_order_relaxed);
        const size_t low = lowPos.load(std::memory_order_acquire);
        if (up >= low + capacity) {
            return -1;
        }
        int readIdx = up % (capacity - 1);
        buffer[readIdx] = x;
        upPos.store(up + 1, std::memory_order_release);
        return 0;
    }

    int pop(T* x) {
        size_t up = upPos.load(std::memory_order_acquire);
        size_t low = lowPos.load(std::memory_order_acquire);
        if (up <= low) {
            // empty queue
            return -1;
        }
        do {
            std::atomic_thread_fence(std::memory_order_seq_cst);
            up = upPos.load(std::memory_order_acquire);
            if (up <= low) {
                return -1;
            }
            int readIdx = low % (capacity - 1);
            *x = buffer[readIdx];
        } while (lowPos.compare_exchange_strong(low, low + 1, 
                                                std::memory_order_seq_cst, 
                                                std::memory_order_relaxed));
        return 0;
    } 

    size_t cap() const { return capacity; }

    size_t volatile_size() const {
        const size_t up = upPos.load(std::memory_order_relaxed);
        const size_t low = lowPos.load(std::memory_order_relaxed);
        return up <= low ? 0 : (up - low);
    }
private:
    //IcQueue(const IcQueue &q) = delete;
    //IcQueue& operator=(const IcQueue&) = delete;

    std::atomic<size_t> upPos;
    std::atomic<size_t> lowPos;
    size_t capacity;
    T* buffer;
};

} // namespace icecream

#endif  // ICECREAM_SRC_LOCK_FREE_QUEUE_H