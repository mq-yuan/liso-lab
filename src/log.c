#include "log.h"
char *IP = "127.0.0.1";
char log_buf[LOG_BUF_SIZE];

void messageLOG(FILE *file, const char *msg, const char *log_type) {
  char date_time[TYPE_SIZE];
  char message[LOG_SIZE];
  data_now(date_time, TYPE_SIZE);
  if (file != NULL) {
    sprintf(message, "%s - - [%s] %s\r\n", IP, date_time, msg);
    fputs(message, file);
    sprintf(message, "[%s] [core:%s] [pid:%d] [client %s] %s\r\n", date_time,
            log_type, getpid(), IP, msg);
    fputs(message, file);
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
