#ifndef PARSE_H
#define PARSE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0

// Header field
typedef struct {
  char header_name[4096];
  char header_value[4096];
} Request_header;

// HTTP Request Header
typedef struct {
  char http_version[50];
  char http_method[50];
  char http_uri[512];
  Request_header *headers;
  int header_count;
} Request;

Request *parse(char *buffer, int size, int socketFd);

// functions decalred in parser.y
int yyparse();
void set_parsing_options(char *buf, size_t i, Request *request);
void yyrestart(FILE *);
#endif
