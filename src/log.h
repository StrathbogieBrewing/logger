#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

// maximum log record string
#define kMaxStrLen (1024)

// maximum log file buffer size
#define kFileBufferSize (10000)
// #define kMaxLogFileSize (kMaxStrLen * kMaxStrLen)

bool log_writeBuffer(time_t t);

bool log_initialise();
bool log_commit();
bool log_terminate();

#endif
