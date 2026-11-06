#include "tnt.h"
#include "backend.h"
#include "tntio.h"

#include <errno.h>

char backend2_device[256];
int backend2_channels;
int backend_primary_channels;
int backend_monitor;
char backend_names[2][16] = {"CB", "HAM"};

static int descriptors[2] = {-1, -1};
static int current_backend;
static int buffer_counts[2] = {-1, -1};

int backend_enabled(void)
{
  return backend2_channels > 0;
}

int backend_validate(int primary_channels, int soft_tnc, int ax25k)
{
  backend_primary_channels = primary_channels;
  if (!backend2_channels && !backend2_device[0])
    return 0;
  if (primary_channels < 1 || primary_channels > 254 ||
      backend2_channels < 1 || backend2_channels > 254 ||
      primary_channels + backend2_channels > 254 ||
      !backend2_device[0] || soft_tnc != 1 || ax25k) {
    fprintf(stderr, "TNT: two backends require Unix hostmode sockets, "
            "positive channel counts and at most 254 total channels\n");
    return 1;
  }
#ifndef TNT_SERIAL
  fprintf(stderr, "TNT: two backends require TNT_SERIAL support\n");
  return 1;
#else
  return 0;
#endif
}

int backend_channel_index(int channel)
{
  if (backend_enabled() && channel > backend_primary_channels && channel != 255)
    return 1;
  return 0;
}

const char *backend_channel_name(int channel)
{
  return backend_names[channel ? backend_channel_index(channel) : backend_monitor];
}

int backend_current(void)
{
  return current_backend;
}

int backend_primary_fd(void)
{
  return descriptors[0];
}

int backend_attach(int primary_fd)
{
  struct sockaddr_un address;
  char discard[256];
  static const char hostmode[] = "\021\030\022\030\033JHOST 1\015";
  int fd;

  if (!backend_enabled())
    return 0;
  if (strlen(backend2_device) >= sizeof(address.sun_path)) {
    fprintf(stderr, "TNT: second backend socket path is too long\n");
    return 1;
  }
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("TNT second backend socket");
    return 1;
  }
  memset(&address, 0, sizeof(address));
  address.sun_family = AF_UNIX;
  strcpy(address.sun_path, backend2_device);
  if (connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
      tnt_write_all(fd, hostmode, sizeof(hostmode) - 1) < 0 ||
      fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
    perror("TNT second backend initialization");
    close(fd);
    return 1;
  }
  sleep(1);
  for (;;) {
    ssize_t count = read(fd, discard, sizeof(discard));
    if (count > 0)
      continue;
    if (count < 0 && errno == EINTR)
      continue;
    if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
      break;
    fprintf(stderr, "TNT: second backend closed during hostmode initialization\n");
    close(fd);
    return 1;
  }
  descriptors[0] = primary_fd;
  descriptors[1] = fd;
  current_backend = 0;
  return 0;
}

int backend_send(const char *data, int length, int target, int *active_fd)
{
  unsigned char packet[259];
  unsigned int channel;
  int local;

  if (length < 4 || length > (int)sizeof(packet)) {
    fprintf(stderr, "TNT: invalid multiplexed hostmode packet length\n");
    return -1;
  }
  channel = (unsigned char)data[0];
  if (channel == 255 || channel > (unsigned int)(backend_primary_channels + backend2_channels) ||
      target < 0 || target > 1) {
    fprintf(stderr, "TNT: invalid multiplexed hostmode channel or backend\n");
    return -1;
  }
  if (channel)
    target = backend_channel_index(channel);
  local = channel;
  if (target && channel)
    local -= backend_primary_channels;
  memcpy(packet, data, length);
  packet[0] = local;
  current_backend = target;
  *active_fd = descriptors[target];
  if (tnt_write_all(*active_fd, packet, length) < 0) {
    fprintf(stderr, "TNT: backend %s write failed\n", backend_names[target]);
    return -1;
  }
  return length;
}

int backend_receive_channel(unsigned int channel)
{
  int limit = current_backend ? backend2_channels : backend_primary_channels;

  if (channel > (unsigned int)limit) {
    fprintf(stderr, "TNT: backend %s returned invalid channel %u\n",
            backend_names[current_backend], channel);
    return -1;
  }
  return channel && current_backend ? channel + backend_primary_channels : channel;
}

int backend_buffer_count(int channel, int primary_count)
{
  return backend_enabled() ? buffer_counts[backend_channel_index(channel)] : primary_count;
}

void backend_set_buffers(int count)
{
  buffer_counts[current_backend] = count;
}

void backend_reserve_buffers(int channel, int count)
{
  int *available = &buffer_counts[backend_channel_index(channel)];
  if (*available >= 0)
    *available -= count;
}

void backend_close(void)
{
  if (descriptors[1] >= 0) {
    static const char terminal[] = {0, 1, 6, 'J', 'H', 'O', 'S', 'T', ' ', '0'};
    if (tnt_write_all(descriptors[1], terminal, sizeof(terminal)) < 0)
      fprintf(stderr, "TNT: second backend terminal-mode request failed\n");
    close(descriptors[1]);
  }
  descriptors[0] = descriptors[1] = -1;
}
