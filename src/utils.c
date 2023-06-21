#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void get_fullpath(char *fullpath, size_t _size, const char *uri) {
  const char *static_site_path = "./static_site";
  const char *default_index_file = "index.html";
  size_t n =
      (_size > strlen(static_site_path)) ? strlen(static_site_path) : _size;
  strncpy(fullpath, static_site_path, n);
  int pos = strpos(uri, '?');
  if (pos != -1) {
    strncat(fullpath, uri, pos);
  } else {
    strcat(fullpath, uri);
  }
  if (is_dir(fullpath)) {
    strcat(fullpath, default_index_file);
  }
}

int is_dir(const char *fullpath) {
  struct stat st;
  if (stat(fullpath, &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
      return 1;
    }
  }
  return 0;
}

int check_file(const char *fullpath) {
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

http_process_result parse_type(const char *fullpath, char *filetype) {
  const char *dot = strrchr(fullpath, '.');
  strcpy(filetype, "Content-type: ");
  if (dot == NULL || strncmp(dot + 1, "cgi", 4) == 0) {
    strcat(filetype, "\r\n");
    return CGI;
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
  return STATIC;
}

void content_length(char *contentlength, const char *fullpath) {
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
  strftime(Data, _size, "Data: %a, %d %b %Y %H:%M:%S %Z", localTime);
}

void data_modify(char *lastmodify, size_t _size, const char *fullpath) {
  struct stat st;
  struct tm *modifyTime;
  if (stat(fullpath, &st) == 0) {
    modifyTime = gmtime(&st.st_mtime);
    strftime(lastmodify, _size, "Last-modified: %a, %d %b %Y %H:%M:%S %Z",
             modifyTime);
  } else {
    strcpy(lastmodify, "");
  }
}

int strpos(const char *str, const char target) {
  int pos, length;
  pos = 0;
  length = strlen(str);
  while (pos < length) {
    if (str[pos] != target) {
      pos++;
    } else {
      return pos;
    }
  }
  return -1;
}

int checkPortInUse(int port) {
  int isUsed = 0;
  char command[50];
  sprintf(command, "lsof -i :%d", port);
  FILE *fp = popen(command, "r");
  if (fp != NULL) {
    char output[1024];
    while (fgets(output, sizeof(output), fp) != NULL) {
      if (strstr(output, "LISTEN") != NULL) {
        isUsed = 1;
        break;
      }
    }
    pclose(fp);
  }
  return isUsed;
}

void releasePort(int port) {
  char command[50];
  sprintf(command, "lsof -t -i :%d | xargs kill", port);
  system(command);
}

int char_split(const char *input, const char *delimiter,
               char regions[MAX_REGIONS][BUF_SIZE]) {
  int count = 0;
  char *token;
  char *rest = (char *)input;

  while ((token = strstr(rest, delimiter)) != NULL && count < MAX_REGIONS) {
    *token = '\0';
    strncpy(regions[count], rest, BUF_SIZE);
    strcat(regions[count], delimiter);
    rest = token + strlen(delimiter);
    count++;
  }

  if (count < MAX_REGIONS) {
    strncpy(regions[count], rest, BUF_SIZE);
    count++;
  }

  return count;
}

void rtrim(char *str) {
  if (str == NULL || *str == '\0')
    return;
  int len = strlen(str);
  char *p = str + len - 1;
  while (p >= str && isspace(*p)) {
    *p = '\0';
    --p;
  }
}

char *newstring(const char *str) {
  char *buf = (char *)malloc(strlen(str) + 1);
  strcpy(buf, str);
  return buf;
}

char *get_header_value(Request *request, const char *header) {
  int i;
  for (i = 0; i < request->header_count; i++) {
    if (!strcmp(request->headers[i].header_name, header)) {
      return request->headers[i].header_value;
    }
  }
  return "";
}

host_and_port *get_hap(const struct sockaddr_in cli_addr) {
  host_and_port *hap = (host_and_port *)malloc(sizeof(host_and_port));
  char remoteIP[INET6_ADDRSTRLEN];
  inet_ntop(cli_addr.sin_family, &cli_addr.sin_addr, remoteIP,
            INET6_ADDRSTRLEN);
  hap->host = newstring(remoteIP);
  hap->port = cli_addr.sin_port;
  return hap;
}

void make_easy_html(char *result, const char *content) {
  sprintf(result,
          "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n    "
          "<title>%s</title>\r\n</head>\r\n<body>\r\n    <h1>%s</h1>\r\n    "
          "</body>\r\n</html>",
          content, content);
}
