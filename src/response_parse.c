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
}

void response_505(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 505 HTTP Version not supported\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
}

void response_501(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 501 Not Implemented\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
}

void response_404(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 404 Bad request\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
}

void response_403(char *buf, size_t _size, ssize_t *readret) {
  memset(buf, 0, _size);
  const char *reply = "HTTP/1.1 403 Forbidden\r\n\r\n";
  size_t n = (_size > strlen(reply)) ? strlen(reply) : _size;
  strncpy(buf, reply, n);
  *readret = strlen(buf);
}

// ===
// === Func For Anything
// ===
void get_fullpath(char *fullpath, size_t _size, char *uri) {
  const char *static_site_path = "./static_site";
  const char *default_index_file = "index.html";
  size_t n =
      (_size > strlen(static_site_path)) ? strlen(static_site_path) : _size;
  strncpy(fullpath, static_site_path, n);
  strcat(fullpath, uri);
  if (is_dir(fullpath)) {
    strcat(fullpath, default_index_file);
  }
}

int is_dir(char *fullpath) {
  struct stat st;
  if (stat(fullpath, &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
      return 1;
    }
  }
  return 0;
}

int check_file(char *fullpath) {
  struct stat st;
  if (stat(fullpath, &st) == 0) {
    if (S_ISREG(st.st_mode)) {
      return 1; // file exist and can access
    } else {
      return 0; // file exists but cannt access
    }
  } else {
    return -1; // file not exist
  }
}

void parse_type(char *fullpath, char *filetype) {
  const char *dot = strrchr(fullpath, '.');
  strcpy(filetype, "Content-type: ");
  if (dot == NULL) {
    strcat(filetype, "\r\n");
    return;
  }
  if (strncmp(dot + 1, "html", 5) == 0) {
    strcat(filetype, "text/html\r\n");
  } else if (strncmp(dot + 1, "css", 4) == 0) {
    strcat(filetype, "text/css\r\n");
  } else if (strncmp(dot + 1, "png", 4) == 0) {
    strcat(filetype, "image/png\r\n");
  } else if (strncmp(dot + 1, "jpeg", 5) == 0) {
    strcat(filetype, "image/jpeg\r\n");
  } else if (strncmp(dot + 1, "gif", 5) == 0) {
    strcat(filetype, "image/gif\r\n");
  } else {
    strcat(filetype, "application/octet-stream\r\n");
  }
}

void content_length(char *contentlength, char *fullpath) {
  struct stat st;
  long length;
  if (stat(fullpath, &st) == 0) {
    length = st.st_size;
  } else {
    length = -1;
  }
  sprintf(contentlength, "Content-Length: %ld\r\n", length);
}

void data_now(char *Data, size_t _size) {
  time_t currenttime;
  struct tm *localTime;
  currenttime = time(NULL);
  localTime = localtime(&currenttime);
  strftime(Data, _size, "Data: %a, %d %b %Y %H:%M:%S %Z\r\n", localTime);
}

void data_modify(char *lastmodify, size_t _size, char *fullpath) {
  struct stat st;
  struct tm *modifyTime;
  if (stat(fullpath, &st) == 0) {
    modifyTime = gmtime(&st.st_mtime);
    strftime(lastmodify, _size, "Last-modified: %a, %d %b %Y %H:%M:%S %Z\r\n",
             modifyTime);
  } else {
    strcpy(lastmodify, "");
  }
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
void response_head(Request *request, char *buf, size_t _size,
                   ssize_t *readret) {
  char fullpath[4096];
  char filetype[64];
  char Data[1024];
  char contentlength[1024];
  char lastmodify[1024];
  const char *statusline = "HTTP/1.1 200 OK\r\n";
  const char *connectline = "Connection: keep-alive\r\n";
  const char *serverline = "Server: liso/1.0\r\n";

  /* parse filetype && content length */
  int status;
  get_fullpath(fullpath, sizeof(fullpath), request->http_uri);
  status = check_file(fullpath);
  switch (status) {
  case -1:
    response_404(buf, _size, readret);
    return;
  case 0:
    response_403(buf, _size, readret);
    return;
  default:
    break;
  }
  parse_type(fullpath, filetype);
  content_length(contentlength, fullpath);

  /* parse now time && last modify time */
  data_now(Data, sizeof(Data));
  data_modify(lastmodify, sizeof(lastmodify), fullpath);

  response_write(buf, _size, readret, statusline, connectline, serverline, Data,
                 filetype, contentlength, lastmodify);
}
