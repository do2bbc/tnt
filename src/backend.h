#ifndef TNT_BACKEND_H
#define TNT_BACKEND_H

extern char backend2_device[256];
extern int backend2_channels;
extern int backend_primary_channels;
extern int backend_monitor;
extern char backend_names[2][16];

int backend_enabled(void);
int backend_validate(int primary_channels, int soft_tnc, int ax25k);
int backend_attach(int primary_fd);
int backend_send(const char *data, int length, int target, int *active_fd);
int backend_receive_channel(unsigned int channel);
int backend_channel_index(int channel);
int backend_current(void);
int backend_primary_fd(void);
int backend_buffer_count(int channel, int primary_count);
void backend_set_buffers(int count);
void backend_reserve_buffers(int channel, int count);
void backend_close(void);
const char *backend_channel_name(int channel);
void cmd_backend(int par1, int par2, int channel, int len, int mode, char *str);

#endif
