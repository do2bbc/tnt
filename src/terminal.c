#include "tnt.h"
#include "terminal.h"
#include <langinfo.h>
#include <locale.h>

static int terminal_utf8;
static unsigned int input_pending;
static char terminal_url[2048];
static size_t terminal_url_length;
static enum { TERMINAL_URL_IDLE, TERMINAL_URL_PREFIX, TERMINAL_URL_ACTIVE }
  terminal_url_state;

static int terminal_plain_write(int fd, const char *buffer, size_t length)
{
  char converted[512];
  size_t used = 0;

  if (!terminal_utf8)
    return tnt_write_all(fd, buffer, length);
  while (length > 0) {
    used += terminal_utf8_output((unsigned char)*buffer++, converted + used);
    --length;
    if (used > sizeof(converted) - 2 || length == 0) {
      if (tnt_write_all(fd, converted, used) < 0)
        return -1;
      used = 0;
    }
  }
  return 0;
}

static int terminal_url_flush(int fd)
{
  static const char open[] = "\033]8;;";
  static const char separator[] = "\033\\";
  static const char close[] = "\033]8;;\033\\";
  char converted_url[sizeof(terminal_url) * 2];
  size_t url_length = terminal_url_length;
  size_t converted_length = 0;
  size_t i;

  if (terminal_url_state == TERMINAL_URL_ACTIVE && url_length > 0) {
    while (url_length > 7 &&
           (terminal_url[url_length - 1] == '.' ||
            terminal_url[url_length - 1] == ',' ||
            terminal_url[url_length - 1] == ';' ||
            terminal_url[url_length - 1] == ':' ||
            terminal_url[url_length - 1] == '!' ||
            terminal_url[url_length - 1] == '?'))
      --url_length;

    for (i = 0; i < url_length; ++i) {
      if (terminal_utf8)
        converted_length += terminal_utf8_output(
          (unsigned char)terminal_url[i], converted_url + converted_length);
      else
        converted_url[converted_length++] = terminal_url[i];
    }

    if (tnt_write_all(fd, open, sizeof(open) - 1) < 0 ||
        tnt_write_all(fd, converted_url, converted_length) < 0 ||
        tnt_write_all(fd, separator, sizeof(separator) - 1) < 0 ||
        tnt_write_all(fd, converted_url, converted_length) < 0 ||
        tnt_write_all(fd, close, sizeof(close) - 1) < 0)
      return -1;
    if (url_length < terminal_url_length &&
        terminal_plain_write(fd, terminal_url + url_length,
                             terminal_url_length - url_length) < 0)
      return -1;
  }
  else if (terminal_url_length > 0 &&
           terminal_plain_write(fd, terminal_url, terminal_url_length) < 0)
    return -1;

  terminal_url_length = 0;
  terminal_url_state = TERMINAL_URL_IDLE;
  return 0;
}

static int terminal_url_process(int fd, unsigned char byte)
{
  static const char http[] = "http://";
  static const char https[] = "https://";

  if (terminal_url_state == TERMINAL_URL_ACTIVE) {
    if (byte <= 0x20 || byte == '"' || byte == '<' || byte == '>' ||
        byte == 0x7f) {
      if (terminal_url_flush(fd) < 0)
        return -1;
      if (byte != 0x7f && terminal_plain_write(fd, (const char *)&byte, 1) < 0)
        return -1;
      return 0;
    }
    if (terminal_url_length >= sizeof(terminal_url) - 1) {
      if (terminal_url_flush(fd) < 0)
        return -1;
      return terminal_plain_write(fd, (const char *)&byte, 1);
    }
    terminal_url[terminal_url_length++] = (char)byte;
    return 0;
  }

  if (terminal_url_state == TERMINAL_URL_PREFIX) {
    terminal_url[terminal_url_length++] = (char)byte;
    if ((terminal_url_length <= sizeof(http) - 1 &&
         strncmp(terminal_url, http, terminal_url_length) == 0) ||
        (terminal_url_length <= sizeof(https) - 1 &&
         strncmp(terminal_url, https, terminal_url_length) == 0)) {
      if (terminal_url_length == sizeof(http) - 1 &&
          memcmp(terminal_url, http, sizeof(http) - 1) == 0)
        terminal_url_state = TERMINAL_URL_ACTIVE;
      else if (terminal_url_length == sizeof(https) - 1 &&
               memcmp(terminal_url, https, sizeof(https) - 1) == 0)
        terminal_url_state = TERMINAL_URL_ACTIVE;
      return 0;
    }

    --terminal_url_length;
    if (terminal_url_flush(fd) < 0)
      return -1;
    return terminal_url_process(fd, byte);
  }

  if (byte == 'h') {
    terminal_url_state = TERMINAL_URL_PREFIX;
    terminal_url_length = 1;
    terminal_url[0] = 'h';
    return 0;
  }
  return terminal_plain_write(fd, (const char *)&byte, 1);
}

void terminal_mouse_tracking(int enabled)
{
  const char *sequence = enabled ? "\033[?1000h\033[?1006h" :
                                   "\033[?1000l\033[?1006l";
  if (terminal_write(1, sequence, strlen(sequence)) < 0)
    perror("TNT mouse tracking");
}

/* Parse the numeric tail after ESC [ <; consume invalid reports to their end. */
int terminal_mouse_parse(struct terminal_mouse *mouse, unsigned char byte)
{
  if (++mouse->bytes > 32) return -1;
  if (byte == 'M' || byte == 'm') {
    if (mouse->invalid || mouse->field != 2 || !mouse->digits ||
        mouse->values[1] == 0 || mouse->values[2] == 0)
      return -1;
    return byte == 'M' ? 1 : -1;
  }
  if (byte >= 0x40 && byte <= 0x7e) return -1;
  if (mouse->invalid) return 0;
  if (byte == ';' && mouse->field < 2 && mouse->digits) {
    ++mouse->field;
    mouse->digits = 0;
  }
  else if (byte >= '0' && byte <= '9') {
    unsigned int value = mouse->values[mouse->field];
    if (value > (65535U - (byte - '0')) / 10)
      mouse->invalid = 1;
    else {
      mouse->values[mouse->field] = value * 10 + byte - '0';
      mouse->digits = 1;
    }
  }
  else
    mouse->invalid = 1;
  return 0;
}

static const char *const palette_roles[] = {
  "normal", "statline", "monitor", "cstatline", "controlchar",
  "remote", "special", "monport1", "monport2"
};
static int foreground[9] = {-1,-1,-1,-1,-1,-1,-1,-1,-1};
static int background[9] = {-1,-1,-1,-1,-1,-1,-1,-1,-1};

static int parse_color(const char *value, char **end)
{
  unsigned long color;
  int rgb = *value == '#';

  if (rgb)
    ++value;
  if (!isxdigit((unsigned char)*value))
    return -1;
  errno = 0;
  color = strtoul(value, end, rgb ? 16 : 10);
  if (errno != 0 || (rgb && *end - value != 6) ||
      color > (rgb ? 0xffffffUL : 255UL))
    return -1;
  return (int)color | (rgb ? 0x1000000 : 0);
}

int terminal_palette_option(const char *name, const char *value)
{
  size_t role;
  char *end;
  int fg, bg;

  for (role = 0; role < sizeof(palette_roles) / sizeof(palette_roles[0]); ++role)
    if (strcmp(name, palette_roles[role]) == 0)
      break;
  if (role == sizeof(palette_roles) / sizeof(palette_roles[0]))
    return 1;
  fg = parse_color(value, &end);
  if (fg < 0 || *end != ',')
    return 1;
  bg = parse_color(end + 1, &end);
  if (bg < 0 || *end != '\0')
    return 1;
  foreground[role] = fg;
  background[role] = bg;
  return 0;
}

static unsigned int palette_rgb(int index)
{
  static const unsigned int ansi[] = {
    0x000000, 0x800000, 0x008000, 0x808000,
    0x000080, 0x800080, 0x008080, 0xc0c0c0,
    0x808080, 0xff0000, 0x00ff00, 0xffff00,
    0x0000ff, 0xff00ff, 0x00ffff, 0xffffff
  };
  static const unsigned int cube[] = {0, 95, 135, 175, 215, 255};
  unsigned int gray;

  if (index < 16)
    return ansi[index];
  if (index < 232) {
    index -= 16;
    return (cube[index / 36] << 16) |
           (cube[(index / 6) % 6] << 8) | cube[index % 6];
  }
  gray = 8 + 10 * (index - 232);
  return gray * 0x010101;
}

static int nearest_color(unsigned int rgb, int colors)
{
  unsigned int best_distance = ~0U;
  int best = 0, index;

  if (colors > 256)
    colors = 256;
  for (index = 0; index < colors; ++index) {
    unsigned int candidate = palette_rgb(index);
    int red = (int)((rgb >> 16) & 255) - (int)((candidate >> 16) & 255);
    int green = (int)((rgb >> 8) & 255) - (int)((candidate >> 8) & 255);
    int blue = (int)(rgb & 255) - (int)(candidate & 255);
    unsigned int distance = red * red + green * green + blue * blue;
    if (distance < best_distance) {
      best_distance = distance;
      best = index;
    }
  }
  return best;
}

static int color_parameter(char *buffer, size_t size, int color, int base,
                           int colors, int truecolor)
{
  if (color & 0x1000000) {
    if (truecolor)
      return snprintf(buffer, size, "%d;2;%d;%d;%d", base + 8,
                      (color >> 16) & 255, (color >> 8) & 255, color & 255);
    color = nearest_color(color & 0xffffff, colors);
  }
  else if (color >= colors)
    color = nearest_color(palette_rgb(color), colors);
  if (color < 8)
    return snprintf(buffer, size, "%d", base + color);
  if (color < 16)
    return snprintf(buffer, size, "%d", base + 60 + color - 8);
  return snprintf(buffer, size, "%d;5;%d", base + 8, color);
}

int terminal_color_sequence(char *buffer, size_t size, int role,
                            unsigned char attribute, int colors, int truecolor)
{
  char fg[32], bg[32];
  int count;

  if (role < 0 || role >= 9 || colors < 8) {
    errno = EINVAL;
    perror("TNT color attributes");
    return -1;
  }
  color_parameter(fg, sizeof(fg),
                  foreground[role] < 0 ? attribute & 7 : foreground[role],
                  30, colors, truecolor);
  color_parameter(bg, sizeof(bg),
                  background[role] < 0 ? (attribute >> 4) & 7 : background[role],
                  40, colors, truecolor);
  count = snprintf(buffer, size, "\033[0;%d;%d;%s;%sm",
                   attribute & 8 ? 1 : 22, attribute & 128 ? 5 : 25, fg, bg);
  if (count < 0 || (size_t)count >= size) {
    errno = EOVERFLOW;
    perror("TNT color attributes");
    return -1;
  }
  return count;
}

void terminal_encoding_init(void)
{
  const char *codeset;

  if (setlocale(LC_CTYPE, "") == NULL) {
    fprintf(stderr, "TNT: invalid character locale; using legacy terminal encoding.\n");
    terminal_utf8 = 0;
  }
  else {
    codeset = nl_langinfo(CODESET);
    terminal_utf8 = strcmp(codeset, "UTF-8") == 0 ||
                    strcmp(codeset, "UTF8") == 0;
  }
  /* Protocol parsers and legacy character tables retain C-locale semantics. */
  setlocale(LC_CTYPE, "C");
  input_pending = 0;
}

size_t terminal_utf8_output(unsigned char byte, char output[2])
{
  if (byte < 0x80) {
    output[0] = byte;
    return 1;
  }
  output[0] = 0xc0 | (byte >> 6);
  output[1] = 0x80 | (byte & 0x3f);
  return 2;
}

int terminal_write(int fd, const char *buffer, size_t length)
{
  size_t i;

  if (fd != 1)
    return terminal_plain_write(fd, buffer, length);
  for (i = 0; i < length; ++i)
    if (terminal_url_process(fd, (unsigned char)buffer[i]) < 0)
      return -1;
  if (terminal_url_state == TERMINAL_URL_ACTIVE &&
      terminal_url_length >= sizeof(terminal_url) - 1 &&
      terminal_url_flush(fd) < 0)
    return -1;
  return 0;
}

/* Return 1 for a legacy byte, 0 while incomplete, -1 for unrepresentable input. */
int terminal_utf8_input(unsigned char byte, unsigned int *pending, char *result)
{
  unsigned int state = *pending;

  if (state != 0) {
    *pending = 0;
    if (byte >= 0x80 && byte <= 0xbf) {
      if (state == 0xc2 || state == 0xc3) {
        *result = ((state & 0x1f) << 6) | (byte & 0x3f);
        return 1;
      }
      if (state > 1)
        *pending = state - 1;
      return state == 1 ? -1 : 0;
    }
    /* Preserve ASCII (notably Escape) after an interrupted UTF-8 sequence. */
    if (byte < 0x80) {
      *result = byte;
      return 1;
    }
    return -1;
  }
  if (byte < 0x80) {
    *result = byte;
    return 1;
  }
  if (byte == 0xc2 || byte == 0xc3) {
    *pending = byte;
    return 0;
  }
  if (byte >= 0xc4 && byte <= 0xdf)
    *pending = 1;
  else if (byte >= 0xe0 && byte <= 0xef)
    *pending = 2;
  else if (byte >= 0xf0 && byte <= 0xf4)
    *pending = 3;
  else
    return -1;
  return 0;
}

int terminal_input(char *ch)
{
  int result;

  if (!terminal_utf8)
    return 1;
  result = terminal_utf8_input((unsigned char)*ch, &input_pending, ch);
  if (result < 0) {
    /* Unicode outside Latin-1 must not become corrupted radio traffic. */
    if (tnt_write_all(2, "\a", 1) < 0)
      return 0;
    return 0;
  }
  return result;
}
