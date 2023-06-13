#ifndef UTILS_h
#define UTILS_h
#include "constants.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void get_fullpath(char *fullpath, size_t _size, const char *uri);
int is_dir(char *fullpath);
int check_file(char *fullpath);
void parse_type(char *fullpath, char *filetype);
void data_now(char *Data, size_t _size);
void data_modify(char *lastmodify, size_t _size, char *fullpath);
void content_length(char *contentlength, char *fullpath);
int strpos(const char *str, const char target);
int char_split(const char *input, const char *delimiter,
               char regions[MAX_REGIONS][BUF_SIZE]);
int checkPortInUse(int port);
void releasePort(int port);

#endif
