#ifndef RESPONSE_PARSE_H
#define RESPONSE_PARSE_H
#include "cgi.h"
#include "constants.h"
#include "log.h"
#include "parse.h"
#include "utils.h"
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
void response_500(char *buf, size_t _size, ssize_t *readret);
void response_write(char *buf, size_t _size, ssize_t *readret,
                    const char *statusline, const char *connectline,
                    const char *serverline, const char *Data,
                    const char *filetype, const char *contentlength,
                    const char *lastmodify);

void response_head(char *fullpath, size_t f_size, Request *request, char *buf,
                   size_t _size, ssize_t *readret, const host_and_port *hap);
void response_get(char *fullpath, size_t f_size, Request *request, char *buf,
                  size_t _size, ssize_t *readret, const host_and_port *hap);
#endif
