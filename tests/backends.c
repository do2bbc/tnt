#include "tnt.h"
#include "backend.h"
#include "tntio.h"

#include <assert.h>
#include <sys/wait.h>

static void check_packet(int fd, int channel, const char *command)
{
  unsigned char packet[259];
  assert(tnt_read_all(fd, packet, 3) == 0);
  assert(packet[0] == channel);
  assert(packet[1] == 1);
  assert(packet[2] + 1 == (int)strlen(command));
  assert(tnt_read_all(fd, packet + 3, packet[2] + 1) == 0);
  assert(memcmp(packet + 3, command, strlen(command)) == 0);
}

int main(int argc, char **argv)
{
  struct sockaddr_un address;
  int primary[2];
  int listener, fd, status, active;
  pid_t child;
  const char first[] = {1, 1, 0, 'I'};
  const char second[] = {11, 1, 0, 'I'};
  const char last[] = {20, 1, 0, 'L'};
  const char zero[] = {0, 1, 0, 'M'};
  const char invalid[] = {(char)255, 1, 0, 'G'};

  assert(argc == 2);
  assert(!backend_enabled());
  assert(backend_validate(10, 1, 0) == 0);
  assert(backend_buffer_count(1, 1200) == 1200);
  assert(strlen(argv[1]) < sizeof(address.sun_path));
  strcpy(backend2_device, argv[1]);
  backend2_channels = 10;
  assert(backend_validate(10, 1, 0) == 0);
  assert(backend_validate(245, 1, 0) != 0);
  assert(backend_validate(10, 0, 0) != 0);
  assert(backend_validate(10, 1, 1) != 0);
  assert(backend_validate(10, 1, 0) == 0);
  assert(backend_channel_index(10) == 0);
  assert(backend_channel_index(11) == 1);
  assert(strcmp(backend_channel_name(10), "CB") == 0);
  assert(strcmp(backend_channel_name(11), "HAM") == 0);

  assert(socketpair(AF_UNIX, SOCK_STREAM, 0, primary) == 0);
  listener = socket(AF_UNIX, SOCK_STREAM, 0);
  assert(listener >= 0);
  memset(&address, 0, sizeof(address));
  address.sun_family = AF_UNIX;
  strcpy(address.sun_path, argv[1]);
  assert(bind(listener, (struct sockaddr *)&address, sizeof(address)) == 0);
  assert(listen(listener, 1) == 0);
  child = fork();
  assert(child >= 0);
  if (!child) {
    char handshake[15];
    static const char expected[] = "\021\030\022\030\033JHOST 1\015";
    fd = accept(listener, NULL, NULL);
    assert(fd >= 0);
    assert(tnt_read_all(fd, handshake, sizeof(expected) - 1) == 0);
    assert(memcmp(handshake, expected, sizeof(expected) - 1) == 0);
    check_packet(fd, 1, "I");
    check_packet(fd, 10, "L");
    check_packet(fd, 0, "M");
    check_packet(fd, 0, "JHOST 0");
    close(fd);
    close(listener);
    close(primary[0]);
    close(primary[1]);
    _exit(0);
  }
  assert(backend_attach(primary[0]) == 0);
  assert(backend_primary_fd() == primary[0]);
  assert(backend_send(first, sizeof(first), 0, &active) == sizeof(first));
  assert(active == primary[0]);
  check_packet(primary[1], 1, "I");
  assert(backend_receive_channel(10) == 10);
  backend_set_buffers(2000);
  assert(backend_send(second, sizeof(second), 0, &active) == sizeof(second));
  assert(active != primary[0]);
  assert(backend_receive_channel(1) == 11);
  assert(backend_receive_channel(10) == 20);
  assert(backend_receive_channel(0) == 0);
  assert(backend_receive_channel(11) == -1);
  assert(backend_receive_channel(255) == -1);
  backend_set_buffers(1000);
  backend_reserve_buffers(11, 20);
  assert(backend_buffer_count(1, 0) == 2000);
  assert(backend_buffer_count(11, 0) == 980);
  assert(backend_send(last, sizeof(last), 0, &active) == sizeof(last));
  assert(backend_send(zero, sizeof(zero), 1, &active) == sizeof(zero));
  assert(backend_send(invalid, sizeof(invalid), 0, &active) == -1);
  backend_close();
  assert(waitpid(child, &status, 0) == child);
  assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
  close(primary[0]);
  close(primary[1]);
  close(listener);
  assert(unlink(argv[1]) == 0);
  return 0;
}
