//
// Created by parkyu on 2023/5/8.
//

#ifndef METALMAX_PERF_H
#define METALMAX_PERF_H

extern "C" void finishPerf(const char * tag);
extern "C" void startPerf();
extern "C" long getDuration();

#endif //METALMAX_PERF_H
