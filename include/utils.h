#ifndef UTILS_h
#define UTILS_h
#include "constants.h"
#include "parse.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

typedef struct host_and_port {
  char *host;
  int port;
} host_and_port;

typedef enum http_process_result { STATIC, CGI } http_process_result;

void get_fullpath(char *fullpath, size_t _size, const char *uri);
int is_dir(const char *fullpath);
int check_file(const char *fullpath);
http_process_result parse_type(const char *fullpath, char *filetype);
void data_now(char *Data, size_t _size);
void data_modify(char *lastmodify, size_t _size, const char *fullpath);
void content_length(char *contentlength, const char *fullpath);
int strpos(const char *str, const char target);
int char_split(const char *input, const char *delimiter,
               char regions[MAX_REGIONS][BUF_SIZE]);
int checkPortInUse(int port);
void releasePort(int port);
void rtrim(char *str);
char *newstring(const char *str);
char *get_header_value(Request *request, const char *header);
host_and_port *get_hap(const struct sockaddr_in cli_addr);

#endif
