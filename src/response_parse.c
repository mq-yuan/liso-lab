#include "response_parse.h"
#include <string.h>

// ===
// === ERROR RESPONSE
// ===
void response_400(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 400 Bad request\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  errorLOG(reply);
}

void response_500(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  errorLOG(reply);
}

void response_505(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  errorLOG(reply);
}

void response_501(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 501 Not Implemented\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  errorLOG(reply);
}

void response_404(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 404 Not Found\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
  errorLOG(reply);
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
                   size_t _size, ssize_t *readret, const host_and_port *hap) {
  int flag = 0; // error flag
  char filetype[TYPE_SIZE];
  char Data[FIELD_SIZE];
  char contentlength[FIELD_SIZE];
  char lastmodify[FIELD_SIZE];
  char token[LOG_BUF_SIZE];
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
    /* strncpy(fullpath, "", strlen("")); */
    return;
  case 0:
    memset(fullpath, 0, f_size);
    response_403(buf, _size, readret);
    /* strncpy(fullpath, "", strlen("")); */
    return;
  default:
    break;
  }
  /* parse now time && last modify time */
  data_now(Data, sizeof(Data));
  strcat(Data, "\r\n");
  data_modify(lastmodify, sizeof(lastmodify), fullpath);
  strcat(lastmodify, "\r\n");
  content_length(contentlength, fullpath);
  if (parse_type(fullpath, filetype) == STATIC) {
    content_length(contentlength, fullpath);
    response_write(buf, _size, readret, statusline, connectline, serverline,
                   Data, filetype, contentlength, lastmodify);
  } else {
    char cgi_result[BUF_SIZE];
    char split_regions[MAX_REGIONS][BUF_SIZE];
    memset(cgi_result, 0, BUF_SIZE);
    CGI_param *cgi_param = build_cgi_param(request, fullpath, *hap);
    handle_cgi(cgi_param, cgi_result);
    int regions_num = char_split(cgi_result, "\r\n", split_regions);
    if (regions_num > 2) {
      memset(filetype, 0, TYPE_SIZE);
      strcat(filetype, split_regions[0]);
      memset(contentlength, 0, TYPE_SIZE);
      strcat(contentlength, split_regions[1]);
      response_write(buf, _size, readret, statusline, connectline, serverline,
                     Data, filetype, contentlength, lastmodify);
      for (int i = 2; i < regions_num; i++) {
        strcat(buf, split_regions[i]);
      }
    } else {
      flag = 1;
      response_500(buf, _size, readret);
    }
    free_CGI_param(cgi_param);
  }
  if (flag == 0) {
    sprintf(token, " \"%s %s %s\" 200 ", request->http_method,
            request->http_uri, request->http_version);
    accessLOG(token);
  }
}

void response_get(char *fullpath, size_t f_size, Request *request, char *buf,
                  size_t _size, ssize_t *readret, const host_and_port *hap) {
  response_head(fullpath, f_size, request, buf, _size, readret, hap);
}
