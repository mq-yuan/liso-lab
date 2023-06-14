#ifndef LOG_H
#define LOG_H

#include "constants.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
extern char log_buf[LOG_BUF_SIZE];
extern char *IP;

void errorLOG(const char *msg);
void accessLOG(const char *msg);
void message(FILE *file, const char *_msg, const char *log_type);
#endif
