#ifndef TNT_IO_H
#define TNT_IO_H

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static inline int tnt_write_all(int fd, const void *buffer, size_t length)
{
  const char *next = buffer;
  while (length > 0) {
    ssize_t count = write(fd, next, length);
    if (count < 0 && errno == EINTR) continue;
    if (count <= 0) {
      if (count == 0) errno = EIO;
      perror("TNT write");
      return -1;
    }
    next += count;
    length -= count;
  }
  return 0;
}

static inline int tnt_read_all(int fd, void *buffer, size_t length)
{
  char *next = buffer;
  while (length > 0) {
    ssize_t count = read(fd, next, length);
    if (count < 0 && errno == EINTR) continue;
    if (count <= 0) {
      if (count == 0) errno = EIO;
      perror("TNT read");
      return -1;
    }
    next += count;
    length -= count;
  }
  return 0;
}

#endif
