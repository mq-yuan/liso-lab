#include "parse.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void response_400(char *buf, size_t _size, ssize_t *readret);
void response_403(char *buf, size_t _size, ssize_t *readret);
void response_404(char *buf, size_t _size, ssize_t *readret);
void response_505(char *buf, size_t _size, ssize_t *readret);
void response_501(char *buf, size_t _size, ssize_t *readret);
void response_write(char *buf, size_t _size, ssize_t *readret,
                    const char *statusline, const char *connectline,
                    const char *serverline, const char *Data,
                    const char *filetype, const char *contentlength,
                    const char *lastmodify);

void get_fullpath(char *fullpath, size_t _size, char *uri);
int is_dir(char *fullpath);
int check_file(char *fullpath);
void parse_type(char *fullpath, char *filetype);
void data_now(char *Data, size_t _size);
void data_modify(char *lastmodify, size_t _size, char *fullpath);
void content_length(char *contentlength, char *fullpath);

void response_head(Request *request, char *buf, size_t _szie, ssize_t *readret);
