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

#include "log.h"
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

namespace icecream {

static std::string logFilename;
static int logFd = -1;
int logFilter = Logger::kLevelDebug;

int initLog(std::string filename, int level) {
    logFilename = filename;
    logFilter = level;
    logFd = open(logFilename.c_str(), O_CREAT | O_APPEND | O_WRONLY | O_NOFOLLOW | O_NOCTTY,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (logFd < 0) {
        return -1;
    }
    return 0;
}

static void Log(int level, const char* file, int line, const char* format, ...) {
    if (level > logFilter) {
        return;
    }
    va_list va;
    va_start(va, format);

    char buffer[48 + 2048];
    switch (level) {
        case Logger::kLevelError   : memcpy(buffer, "ERROR ", 6); break;
        case Logger::kLevelWarn    : memcpy(buffer, "WARN  ", 6); break;
        case Logger::kLevelInfo    : memcpy(buffer, "INFO  ", 6); break;
        case Logger::kLevelDebug   : memcpy(buffer, "DEBUG ", 6); break;
        default                    : memcpy(buffer, "OTHER ", 6); break;
    };

    long tid = getpid();
    struct tm tm;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);
    ssize_t sret = sprintf(buffer + 6,
                   "%04d-%02d-%02d %02d:%02d:%02d.%06ld %" "6ld",
                   tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday,
                   tm.tm_hour, tm.tm_min, tm.tm_sec,
                   static_cast<long>(tv.tv_usec),
                   tid);
    buffer[6 + 26] = ' ';
    char* next = buffer + 6 + 26 + 1;
    sret = vsnprintf(next, 2048, format, va);
    if (sret < 0) {
        return;
    } else if (static_cast<size_t>(sret) > 2048) {
        memset(next + 2048 - 3, '.', 3);
        sret = 2048;
    }
    size_t ret = static_cast<size_t>(sret);
    next += ret;
    *next++ = '\n';
    ret = static_cast<size_t>(next - buffer);
    int writeRes = write(logFd, buffer, ret);
    va_end(va);
    return;

}

Logger::~Logger() {
    std::string s = _buffer.str();
    Log(_level, _file, _line, "%s", s.c_str());
}

}