#include "log.h"
#include <string.h>
char *IP = "127.0.0.1";
char log_buf[LOG_BUF_SIZE];

void messageLOG(FILE *file, const char *_msg, const char *log_type) {
  char date_time[FIELD_SIZE];
  char message[LOG_SIZE];
  char msg[LOG_BUF_SIZE];
  size_t n = (LOG_BUF_SIZE > strlen(_msg)) ? strlen(_msg) : LOG_BUF_SIZE;
  strncpy(msg, _msg, n);
  data_now(date_time, FIELD_SIZE);
  rtrim(msg);
  if (file != NULL) {
    if (strncmp(log_type, "access", 7) == 0) {
      sprintf(message, "%s - - [%s] %s\r\n", IP, date_time, msg);
      fputs(message, file);
    } else {
      sprintf(message, "[%s] [core:%s] [pid:%d] [client %s] %s\r\n", date_time,
              log_type, getpid(), IP, msg);
      fputs(message, file);
    }
    fclose(file);
  }
  memset(log_buf, 0, LOG_BUF_SIZE);
}
void errorLOG(const char *msg) {
  FILE *file = fopen("error_log.log", "a+");
  messageLOG(file, msg, "error");
}

void accessLOG(const char *msg) {
  FILE *file = fopen("access_log.log", "a+");
  messageLOG(file, msg, "access");
}
