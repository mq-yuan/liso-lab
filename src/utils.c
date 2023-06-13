#include "utils.h"

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

int is_dir(char *fullpath) {
  struct stat st;
  if (stat(fullpath, &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
      return 1;
    }
  }
  return 0;
}

int check_file(char *fullpath) {
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

void parse_type(char *fullpath, char *filetype) {
  const char *dot = strrchr(fullpath, '.');
  strcpy(filetype, "Content-type: ");
  if (dot == NULL) {
    strcat(filetype, "\r\n");
    return;
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
}

void content_length(char *contentlength, char *fullpath) {
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

void data_modify(char *lastmodify, size_t _size, char *fullpath) {
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
    strcat(regions[count], "\r\n\r\n");
    rest = token + strlen(delimiter);
    count++;
  }

  if (count < MAX_REGIONS) {
    strncpy(regions[count], rest, BUF_SIZE);
    count++;
  }

  return count;
}
