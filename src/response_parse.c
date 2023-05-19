#include "response_parse.h"

void response_400(char *buf, size_t _size, ssize_t *readret) {
  const char *reply = "HTTP/1.1 400 Bad request\r\n\r\n";
  size_t n = (_size > strlen(reply) + 1) ? strlen(reply) + 1 : _size;
  strncpy(buf, reply, n);
  buf[n - 1] = '\0';
  *readret = strlen(buf);
}

void response_505(char *buf, size_t _size, ssize_t *readret) {
  const char *reply = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
  size_t n = (_size > strlen(reply) + 1) ? strlen(reply) + 1 : _size;
  strncpy(buf, reply, n);
  buf[n - 1] = '\0';
  *readret = strlen(buf);
}

void response_501(char *buf, size_t _size, ssize_t *readret) {
  const char *reply = "HTTP/1.1 501 Not Implemented\r\n\r\n";
  size_t n = (_size > strlen(reply) + 1) ? strlen(reply) + 1 : _size;
  strncpy(buf, reply, n);
  buf[n - 1] = '\0';
  *readret = strlen(buf);
}
