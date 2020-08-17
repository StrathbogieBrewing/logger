#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "log.h"
#include "mkdir.h"
#include "udp.h"

// #define STRINGIZE_DETAIL(x) #x
// #define STRINGIZE(x) STRINGIZE_DETAIL(x)
// #define LINE STRINGIZE(__LINE__)

// maximum log record string
#define kMaxStrLen (1024)

// maximum log file buffer size
#define kMaxLogFileSize (kMaxStrLen * kMaxStrLen)

// listening port
#define kUDPPort (12345)

// #define kMaxLogFileSize (USHRT_MAX)
// static char *fileBuffer;
// static unsigned short fileBufferIndex = 0;

static char *loggerDirectory = "";

// static char filePath[LOG_MAX_STRING_LEN];

static volatile int keepRunning = 1;
void intHandler(int dummy) { keepRunning = 0; }

int main(int argc, char *argv[]) {

  log_initialise("/home/johny");

  struct timeval tv1;
  gettimeofday(&tv1, NULL);
  // suseconds_t ms = tv.tv_usec / 1000L;
  // struct tm tm = *gmtime(&tv.tv_sec

  log_writeBuffer(tv1.tv_sec);

  return 0;

  signal(SIGINT, intHandler);

  int opt;

  opterr = 0;

  while ((opt = getopt(argc, argv, "d:")) != -1)
    switch (opt) {

    case 'd':
      loggerDirectory = optarg;
      break;
    case '?':
      if (optopt == 'd')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      return 1;
    default:
      abort();
    }

  printf("directory = %s\n", loggerDirectory);

  for (int index = optind; index < argc; index++)
    printf("Non-option argument %s\n", argv[index]);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  suseconds_t ms = tv.tv_usec / 1000L;
  struct tm tm = *gmtime(&tv.tv_sec);

  // printf("%s%4.4ld/%2.2ld/%2.2ld/%2.2ld/%2.2ld\n", loggerDirectory,
  //        1900L + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour,
  //        tm.tm_min);

  char path[256];
  sprintf(path, "%s%4.4ld/%2.2ld/%2.2ld/%2.2ld", loggerDirectory,
          1900L + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour);

  printf("%s\n", path);
  build(path);

  return 0;

  // char rxString[udp_kBufferSize] = {0};
  //
  // if (argc != 2) {
  //   fprintf(stdout, "Usage: %s <logpath>\n", argv[0]);
  //   return EXIT_FAILURE;
  // }
  // if (strlen(argv[1]) > LOG_MAX_STRING_LEN - 16) {
  //   fprintf(stdout, "Path too long\n");
  //   return EXIT_FAILURE;
  // }
  //
  // strncpy(filePath, argv[1], LOG_MAX_STRING_LEN);
  //
  // return 0;
  //
  // fileBuffer = (char *)malloc(kMaxLogFileSize);
  // if (fileBuffer == NULL) {
  //   fprintf(stderr, "Could not allocate required memory\n");
  //   return EXIT_FAILURE;
  // }
  //
  // if (udp_open(kUDPPort) == -1) {
  //   perror(__FILE__ " Line " LINE);
  //   return EXIT_FAILURE;
  // }
  //
  // while (keepRunning) {
  //   // process udp packets with blocking read
  //   int rc = udp_read(rxString);
  //   if (rc > 0) {
  //     // remove cr / lf and terminate
  //     int len = strlen(rxString);
  //     int i = 0;
  //     while (i < len) {
  //       if (rxString[i] == '\r')
  //         rxString[i] = '\0';
  //       if (rxString[i] == '\n')
  //         rxString[i] = '\0';
  //       i++;
  //     }
  //     putLog(rxString);
  //   } else if (rc < 0) {
  //     // record bad datagram
  //     fprintf(stderr, "Log UDP Packet Error\n");
  //   }
  // }
  //
  // // tear down db and udp port
  // putLog(NULL);
  // udp_close();

  return EXIT_SUCCESS;
}
