#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "udp.h"

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define LINE STRINGIZE(__LINE__)

// log record structure
#define LOG_MAX_STRING_LEN (udp_kBufferSize)

// listening port
#define kUDPPort (12345)

#define kMaxLogFileSize (USHRT_MAX)
static char *fileBuffer;
static unsigned short fileBufferIndex = 0;

static char filePath[LOG_MAX_STRING_LEN];

static volatile int keepRunning = 1;
void intHandler(int dummy) { keepRunning = 0; }

void putLog(char *logString) {
  static int bufferMinute = -1;
  static char fileName[LOG_MAX_STRING_LEN] = {0};

  if (logString == NULL) {
    // write buffer
    FILE *fd = fopen(fileName, "a");
    if (fd != NULL) {
      fputs(fileBuffer, fd);
      fclose(fd);
    }
    return;
  }

  struct timeval tv;
  gettimeofday(&tv, NULL);
  suseconds_t ms = tv.tv_usec / 1000L;
  struct tm tm = *gmtime(&tv.tv_sec);

  int minute = tm.tm_min + tm.tm_hour * 60 + tm.tm_yday * 60 * 24;

  char logTime[LOG_MAX_STRING_LEN];
  if (tm.tm_sec) {
    sprintf(logTime, "%ld%3.3ld", tm.tm_sec, ms);
  } else {
    sprintf(logTime, "%3.3ld", ms);
  }

  char logEntry[LOG_MAX_STRING_LEN];
  sprintf(logEntry, "{\"t\":%s,%s}\n", logTime, logString);

  // initialise bufferMinute and fileName on first call
  if (bufferMinute < 0) {
    bufferMinute = minute;
    sprintf(fileName, "%s%4.4ld%2.2ld%2.2ld%2.2ld.json", filePath,
            1900L + tm.tm_year, tm.tm_mday, tm.tm_hour, tm.tm_min);
  }

  if (bufferMinute != minute) {
    // write buffer
    FILE *fd = NULL;
    fd = fopen(fileName, "a");
    if (fd != NULL) {
      fputs(fileBuffer, fd);
      fclose(fd);
    }
    // initialise for next file
    fileBufferIndex = 0;
    bufferMinute = minute;
    sprintf(fileName, "%s%4.4ld%2.2ld%2.2ld%2.2ld.json", filePath,
            1900L + tm.tm_year, tm.tm_mday, tm.tm_hour, tm.tm_min);
  }

  strcpy(&fileBuffer[fileBufferIndex], logEntry);
  fileBufferIndex += strlen(logEntry);

  printf("%s : %s", fileName, logEntry);
}

int main(int argc, char *argv[]) {
  char rxString[udp_kBufferSize] = {0};

  if (argc != 2) {
    fprintf(stdout, "Usage: %s <logpath>\n", argv[0]);
    return EXIT_FAILURE;
  }
  if (strlen(argv[1]) > LOG_MAX_STRING_LEN - 16) {
    fprintf(stdout, "Path too long\n");
    return EXIT_FAILURE;
  }

  strncpy(filePath, argv[1], LOG_MAX_STRING_LEN);

  signal(SIGINT, intHandler);

  fileBuffer = (char *)malloc(kMaxLogFileSize);
  if (fileBuffer == NULL) {
    fprintf(stdout, "Could not allocate required memory\n");
    return EXIT_FAILURE;
  }

  if (udp_open(kUDPPort) == -1) {
    perror(__FILE__ " Line " LINE);
    return EXIT_FAILURE;
  }

  while (keepRunning) {
    // process udp packets with blocking read
    int rc = udp_read(rxString);
    if (rc > 0) {
      // remove cr / lf and terminate
      int len = strlen(rxString);
      int i = 0;
      while (i < len) {
        if (rxString[i] == '\r')
          rxString[i] = '\0';
        if (rxString[i] == '\n')
          rxString[i] = '\0';
        i++;
      }
      putLog(rxString);
    } else if (rc < 0) {
      // record bad datagram
      fprintf(stderr, "Log UDP Packet Error\n");
    }
  }

  // tear down db and udp port
  putLog(NULL);
  udp_close();
  free(fileBuffer);

  return EXIT_SUCCESS;
}
