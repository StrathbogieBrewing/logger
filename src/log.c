#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "log.h"
#include "mkdir.h"

typedef struct {
  short unsigned milliSeconds;
  char id[2];
  float value;
} logRecord_t;

// // time stamp is formatted as MM:SS.MS
// #define kTimeStampStrLen (sizeof("MM:SS.mmm"))

static logRecord_t fileBuffer[kFileBufferSize];
static long fileBufferIndex = 0;
static time_t fileBufferTime = 0;

static char basePath[kMaxStrLen] = {0};

time_t log_getMinutesFromEpoch(struct timeval *tv) { return tv->tv_sec / 60L; }

int log_getMinutesOfHour(struct timeval *tv) {
  time_t seconds = tv->tv_sec;
  struct tm tm = *gmtime(&seconds);
  return tm.tm_min;
}

static bool isSameMinute(time_t t1, time_t t2) {
  struct tm tm1 = *gmtime(&t1);
  struct tm tm2 = *gmtime(&t2);
  if (tm1.tm_min == tm2.tm_min)
    return true;
  return false;
}

unsigned short log_getMillis(struct timeval *tv) {
  suseconds_t millis = tv->tv_usec / 1000L;
  time_t seconds = tv->tv_sec;
  struct tm tm = *gmtime(&seconds);
  return tm.tm_sec * 1000 + millis;
}

static int log_getTimeStampStr(char *dst, struct timeval *tv) {
  suseconds_t millis = tv->tv_usec / 1000L;
  time_t seconds = tv->tv_sec;
  struct tm tm = *gmtime(&seconds);
  sprintf(dst, "%2.2u:%2.2u.%3.3u", tm.tm_min, tm.tm_sec, millis);
  return strlen(dst);
}

bool log_writeBuffer(time_t t) {
  struct tm tm = *gmtime(&t);
  char directory[kMaxStrLen];
  sprintf(directory, "%s%4.4u/%2.2u/%2.2u/%2.2u", basePath, 1900L + tm.tm_year,
          tm.tm_mon, tm.tm_mday, tm.tm_hour);
  // build(directory);
  char destination[kMaxStrLen];
  sprintf(destination, "%s/%2.2u.bin", directory, tm.tm_min);

  puts(destination);
  // FILE *fd = fopen(destination, "w");
  // if (fd != NULL) {
  //   fwrite(fileBuffer, sizeof(logRecord_t), fileBufferIndex, fd);
  //   fclose(fd);
  //   return true;
  // }
  return false;
}

bool log_initialise(const char *path) {
  fileBufferIndex = 0;
  fileBufferTime = 0;
  int length = strnlen(path, kMaxStrLen);
  if (length >= kMaxStrLen)
    return false;
  strncpy(basePath, path, kMaxStrLen);
  if (length) {
    if (basePath[length - 1] != '/')
      strncat(basePath, "/", kMaxStrLen - 1);
  }
  return true;
}

bool log_commit(const char *logString) {
  struct timeval tv;
  char logEntry[kMaxStrLen];
  char timeStamp[kMaxStrLen];

  if (logString == NULL)
    return false;
  if (*logString == '\0')
    return false;
  int length = strnlen(logString, kMaxStrLen);
  if (length >= kMaxStrLen)
    return false;

  // get timestamp for these log entries
  gettimeofday(&tv, NULL);
  time_t timeNow = tv.tv_sec;

  struct tm tm = *gmtime(&tv.tv_sec);
  unsigned short millisNow = (tv.tv_usec / 1000) + (tm.tm_sec * 1000);

  if (fileBufferTime == 0)
    fileBufferTime = timeNow;

  if (!isSameMinute(fileBufferTime, timeNow)) {
    // save buffer and start new minute buffer log
    log_writeBuffer(fileBufferTime);
    fileBufferIndex = 0;
    fileBufferTime = timeNow;
  }

  // log_getTimeStampStr(timeStamp, &tv);

  // parse the log string
  // **=float{,**=}

  // snprintf(logEntry, )
  //
  //     // append time stamp and log string to buffer
  //
  //     fileBuffer[fileBufferIndex]
  //         .milliSeconds =
  //
  //     int newIndex = fileBufferIndex + length;
  // if (newIndex > kFileBufferSize)
  //   return false;
  // memcpy(&fileBuffer[fileBufferIndex], logString, length);
  // fileBufferIndex = newIndex;

  return true;
}

// bool putLog(char *logString) {
//   static int bufferMinute = -1;
//   static char file[kMaxStrLen] = {0};
//   static char path[kMaxStrLen] = {0};
//
//   static char *fileBuffer = NULL;
//   static unsigned short fileBufferIndex = 0;
//
//   if (fileBuffer == NULL) {
//     fileBuffer = (char *)malloc(kMaxLogFileSize);
//     if (fileBuffer == NULL) {
//       fprintf(stderr, "Could not allocate required memory\n");
//       return false;
//     }
//   }
//
//   if (logString == NULL) {
//     if (fileBuffer != NULL)
//       if (bufferMinute >= 0) {
//         // write buffer to file
//         log_write(file, path, fileBuffer);
//         return;
//       }
//     free(fileBuffer);
//   }
//
//    // && (bufferMinute >= 0)) {
//    //   // write buffer to file
//    //   writeFile(file, path, fileBuffer);
//    //   return;
//    // }
//
//    struct timeval tv;
//    gettimeofday(&tv, NULL);
//    suseconds_t ms = tv.tv_usec / 1000L;
//    struct tm tm = *gmtime(&tv.tv_sec);
//
//    int minute = tm.tm_min + tm.tm_hour * 60 + tm.tm_yday * 60 * 24;
//
//    char logTime[LOG_MAX_STRING_LEN];
//
//    // if (tm.tm_sec) {
//    sprintf(logTime, "%2.2ld.%3.3ld", tm.tm_sec, ms);
//    // } else {
//    //   sprintf(logTime, "%3.3ld", ms);
//    // }
//
//    char logEntry[LOG_MAX_STRING_LEN];
//    sprintf(logEntry, "%s,%s}\n", logTime, logString);
//
//    // initialise bufferMinute and fileName on first call
//    if (bufferMinute < 0) {
//      bufferMinute = minute;
//      sprintf(fileName, "%s%4.4ld%2.2ld%2.2ld%2.2ld.json", filePath,
//              1900L + tm.tm_year, tm.tm_mday, tm.tm_hour, tm.tm_min);
//    }
//
//    if (bufferMinute != minute) {
//      // write buffer
//      log_write(fileName, path, fileBuffer);
//
//      FILE *fd = NULL;
//      fd = fopen(fileName, "a");
//      if (fd != NULL) {
//        fputs(fileBuffer, fd);
//        fclose(fd);
//      }
//      // initialise for next file
//      fileBufferIndex = 0;
//      bufferMinute = minute;
//      sprintf(fileName, "%s%4.4ld%2.2ld%2.2ld%2.2ld.json", filePath,
//              1900L + tm.tm_year, tm.tm_mday, tm.tm_hour, tm.tm_min);
//    }
//
//    // struct timeval tv;
//    // gettimeofday(&tv, NULL);
//    // suseconds_t ms = tv.tv_usec / 1000L;
//    // struct tm tm = *gmtime(&tv.tv_sec);
//    strcpy(&fileBuffer[fileBufferIndex], logEntry);
//    fileBufferIndex += strlen(logEntry);
//
//    printf("%s : %s", fileName, logEntry);
// }

bool log_terminate() {
  // if (fileBuffer != NULL) {
  //   if (logTime != 0) {
  //   }
  //   free(fileBuffer);
  // }
  return true;
};
