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

#ifndef ICECREAM_SRC_LOG_H
#define ICECREAM_SRC_LOG_H

#include <iostream>
#include <sstream>

namespace icecream {
// #define log(N) std::cout

extern int logFilter;

extern int initLog(std::string filename, int level);

class Logger{
public:
    enum Level {
        kLevelError   = 0,
        kLevelWarn    = 1,
        kLevelInfo    = 2,
        kLevelDebug   = 3,

    }; // enum Level
    Logger(const char *file, int line, const Level &level)
            : _file(file), _line(line), _level(level) {}

    virtual ~Logger();

    template <class T>
    Logger &operator <<(const T &t)
    {
        if (_level > logFilter) {
            return *this;
        }
        _buffer << t;
        return *this;
    }

private:
    std::ostringstream _buffer;
    Level _level;
    const char *_file;
    int _line;

}; // class Logger

} // namespace icecream

#define LOG_IMPL(x)    (::icecream::Logger(__FILE__, __LINE__, x))
#define LOG_ERROR      LOG_IMPL(::icecream::Logger::kLevelError)
#define LOG_WARN       LOG_IMPL(::icecream::Logger::kLevelWarn)
#define LOG_INFO       LOG_IMPL(::icecream::Logger::kLevelInfo)
#define LOG_DEBUG      LOG_IMPL(::icecream::Logger::kLevelDebug)

#define LOG(x) LOG_##x
#define log(x) LOG_##x

#endif // ICECREAM_SRC_LOG_H