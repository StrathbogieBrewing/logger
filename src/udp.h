#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define udp_kBufferSize 512

int udp_open(int udp_port);
int udp_read(char *datagram);
void udp_close(void);

#if defined(__cplusplus)
}
#endif

#endif
