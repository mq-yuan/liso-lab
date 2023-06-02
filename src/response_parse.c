#include "response_parse.h"

// ===
// === ERROR RESPONSE
// ===
void response_400(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 400 Bad request\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  accessLOG(reply);
}

void response_505(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  accessLOG(reply);
}

void response_501(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 501 Not Implemented\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  accessLOG(reply);
}

void response_404(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 404 Not Found\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  accessLOG(reply);
}

void response_403(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 403 Forbidden\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  errorLOG(reply);
}

void response_write(char *buf, size_t _size, ssize_t *readret,
                    const char *statusline, const char *connectline,
                    const char *serverline, const char *Data,
                    const char *filetype, const char *contentlength,
                    const char *lastmodify) {
  memset(buf, 0, _size);
  strcpy(buf, statusline);
  strcat(buf, connectline);
  strcat(buf, serverline);
  strcat(buf, Data);
  strcat(buf, filetype);
  strcat(buf, contentlength);
  strcat(buf, lastmodify);
  strcat(buf, "\r\n");
  *readret = strlen(buf);
}
// ===
// === HEAD && GET
// ===
void response_head(char *fullpath, size_t f_size, Request *request, char *buf,
                   size_t _size, ssize_t *readret) {
  char filetype[64];
  char Data[1024];
  char contentlength[1024];
  char lastmodify[1024];
  const char *statusline = "HTTP/1.1 200 OK\r\n";
  const char *connectline = "Connection: keep-alive\r\n";
  const char *serverline = "Server: liso/1.0\r\n";

  /* parse filetype && content length */
  int status;
  get_fullpath(fullpath, f_size, request->http_uri);
  status = check_file(fullpath);
  switch (status) {
  case -1:
    memset(fullpath, 0, f_size);
    response_404(buf, _size, readret);
    strncpy(fullpath, "", strlen(""));
    return;
  case 0:
    memset(fullpath, 0, f_size);
    response_403(buf, _size, readret);
    strncpy(fullpath, "", strlen(""));
    return;
  default:
    break;
  }
  parse_type(fullpath, filetype);
  content_length(contentlength, fullpath);

  /* parse now time && last modify time */
  data_now(Data, sizeof(Data));
  strcat(Data, "\r\n");
  data_modify(lastmodify, sizeof(lastmodify), fullpath);
  strcat(lastmodify, "\r\n");

  response_write(buf, _size, readret, statusline, connectline, serverline, Data,
                 filetype, contentlength, lastmodify);
}

void response_get(char *fullpath, size_t f_size, Request *request, char *buf,
                  size_t _size, ssize_t *readret) {
  response_head(fullpath, f_size, request, buf, _size, readret);
}
