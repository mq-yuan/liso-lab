#ifndef LOG_H
#define LOG_H
#define LOG_BUF_SIZE 1024
#define TYPE_SIZE 4096
#define LOG_SIZE 4096

#include "utils.h"
#include <stdio.h>
#include <unistd.h>
char log_buf[LOG_BUF_SIZE];
extern char *IP;

void errorLOG(const char *msg);
void accessLOG(const char *msg);
void message(FILE *file, const char *msg, const char *log_type);
#endif
