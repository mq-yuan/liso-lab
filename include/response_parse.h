#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void response_400(char *buf, size_t _size, ssize_t *readret);
void response_505(char *buf, size_t _size, ssize_t *readret);
void response_501(char *buf, size_t _size, ssize_t *readret);
