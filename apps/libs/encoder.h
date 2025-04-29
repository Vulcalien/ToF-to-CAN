/*
 * encoder.h
 */

#ifndef __ENCODER_H
#define __ENCODER_H

#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <nuttx/sensors/qencoder.h>

class Encoder {
 public:
    Encoder(const char * path) {
        fd = open(path, O_RDONLY);
        if (fd < 0)
            perror("Encoder: Error opening device");
    };
    ~Encoder() {
        close(fd);
    };
    int reset() {  return ioctl(fd, QEIOC_RESET, 0); };
    void read(int32_t & position) {
        int ret = ioctl(fd, QEIOC_POSITION, (unsigned long)((uintptr_t)&position));
        if (ret < 0)
            perror("Encoder: ioctl(QEIOC_POSITION) failed");
    };
 private:
    int fd;
};

#endif
