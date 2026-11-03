#ifndef TNT_TERMINAL_H
#define TNT_TERMINAL_H

#include <stddef.h>

void terminal_encoding_init(void);
int terminal_write(int fd, const char *buffer, size_t length);
int terminal_input(char *ch);
struct terminal_mouse {
  unsigned int values[3];
  unsigned int field;
  int digits;
  int invalid;
  unsigned int bytes;
};
int terminal_mouse_parse(struct terminal_mouse *mouse, unsigned char byte);
void terminal_mouse_tracking(int enabled);
int terminal_utf8_input(unsigned char byte, unsigned int *pending, char *result);
size_t terminal_utf8_output(unsigned char byte, char output[2]);
int terminal_palette_option(const char *name, const char *value);
int terminal_color_sequence(char *buffer, size_t size, int role,
                            unsigned char attribute, int colors, int truecolor);

#endif
