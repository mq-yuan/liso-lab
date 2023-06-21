#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIELD_SIZE 1024
#define BUF_SIZE 8192

void cleanup() { printf("HTTP/1.1 500 Internal Server Error"); }

int main() {
  atexit(cleanup);

  char *request_method = getenv("REQUEST_METHOD");
  if (request_method == NULL) {
    exit(1);
  }
  if (!strncmp(request_method, "GET", 4)) {
    char *query_str = getenv("QUERY_STRING");
    if (query_str == NULL) {
      exit(1);
    }
    char username[FIELD_SIZE];
    char password[FIELD_SIZE];
    char content[BUF_SIZE];

    char *username_start, *username_end, *passwd_start, *passwd_end;
    username_start = strchr(query_str, '=') + 1;
    username_end = strchr(query_str, '&');
    strncpy(username, username_start, username_end - username_start);
    username[username_end - username_start] = '\0';
    passwd_start = strchr(username_end + 1, '=') + 1;
    passwd_end = strchr(username_end + 1, '\0');
    strncpy(password, passwd_start, passwd_end - passwd_start);
    password[passwd_end - passwd_start] = '\0';

    /* error test */
    if (!strncmp(username, "error", 6)) {
      int *ptr = NULL;
      *ptr = 10;
      return 0;
    }

    /* print result */
    printf("Content-type: text/html\r\n");
    sprintf(content, "Username: %s\r\nPassword: %s\r\n", username, password);
    printf("Content-length: %lu\r\n", strlen(content));
    printf("%s", content);
  } else if (!strncmp(request_method, "POST", 5)) {
    exit(1);
  }
  return 0;
}
