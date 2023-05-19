/******************************************************************************
 * echo_server.c                                                               *
 *                                                                             *
 * Description: This file contains the C source code for an echo server.  The  *
 *              server runs on a hard-coded port and simply write back anything*
 *              sent to it by connected clients.  It does not support          *
 *              concurrent clients.                                            *
 *                                                                             *
 * Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
 *          Wolf Richter <wolf@cs.cmu.edu>                                     *
 *                                                                             *
 *******************************************************************************/

#include "parse.h"
#include "response_parse.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int sock, client_sock;

int close_socket(int sock) {
  if (close(sock)) {
    fprintf(stderr, "Failed closing socket.\n");
    return 1;
  }
  return 0;
}

void signal_handler(int signum) {
  close_socket(sock);
  exit(signum);
}

int main(int argc, char *argv[]) {
  int echo_port;
  ssize_t readret;
  socklen_t cli_size;
  struct sockaddr_in addr, cli_addr;
  char buf[BUF_SIZE];

  if (argc == 2) {
    echo_port = atoi(argv[1]);
  } else {
    echo_port = ECHO_PORT;
  }
  signal(SIGINT, signal_handler);

  fprintf(stdout, "----- Echo Server -----\n");

  /* all networked programs must create a socket */
  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "Failed creating socket.\n");
    return EXIT_FAILURE;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(echo_port);
  addr.sin_addr.s_addr = INADDR_ANY;

  /* servers bind sockets to ports---notify the OS they accept connections */
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr))) {
    close_socket(sock);
    fprintf(stderr, "Failed binding socket.\n");
    return EXIT_FAILURE;
  }

  if (listen(sock, 5)) {
    close_socket(sock);
    fprintf(stderr, "Error listening on socket.\n");
    return EXIT_FAILURE;
  }

  /* finally, loop waiting for input and then write it back */
  while (1) {
    cli_size = sizeof(cli_addr);
    if ((client_sock = accept(sock, (struct sockaddr *)&cli_addr, &cli_size)) ==
        -1) {
      close(sock);
      fprintf(stderr, "Error accepting connection.\n");
      return EXIT_FAILURE;
    }

    readret = 0;

    while ((readret = recv(client_sock, buf, BUF_SIZE, 0)) >= 1) {
      /* Parsing */
      Request *request = parse(buf, BUF_SIZE, client_sock);

      /* Generating the Response Messages for Unimplemented and Formatting
       * Errors*/
      /* For NULL */
      if (request == NULL) {
        memset(buf, 0, BUF_SIZE);
        response_400(buf, sizeof(buf), &readret);
        /* early implement
const char *reply = "HTTP/1.1 400 Bad request\r\n\r\n";
size_t n =
    (sizeof(buf) > strlen(reply) + 1) ? strlen(reply) + 1 : sizeof(buf);
strncpy(buf, reply, n);
buf[n - 1] = '\0';
readret = strlen(buf);
        */
      }
      /* For HTTP VERSION */
      else if (strncmp(request->http_version, "HTTP/1.1", 9) != 0) {
        memset(buf, 0, BUF_SIZE);
        response_505(buf, sizeof(buf), &readret);
        /* early implement
const char *reply = "HTTP/1.1 505 HTTP Version Not Support\r\n\r\n";
size_t n =
    (sizeof(buf) > strlen(reply) + 1) ? strlen(reply) + 1 : sizeof(buf);
strncpy(buf, reply, n);
buf[n - 1] = '\0';
readret = strlen(buf);
        */
      }
      /* FOR NO IMPLEMENT */
      else if ((strncmp(request->http_method, "GET", 4) != 0) &&
               (strncmp(request->http_method, "POST", 5) != 0) &&
               (strncmp(request->http_method, "HEAD", 5) != 0)) {
        memset(buf, 0, BUF_SIZE);
        response_501(buf, sizeof(buf), &readret);
        /* early implement
const char *reply = "HTTP/1.1 501 Not Implemented\r\n\r\n";
size_t n =
    (sizeof(buf) > strlen(reply) + 1) ? strlen(reply) + 1 : sizeof(buf);
strncpy(buf, reply, n);
buf[n - 1] = '\0';
readret = strlen(buf);
        */
      }
      /* FOR POST */
      else if ((strncmp(request->http_method, "POST", 5) == 0)) {

      }
      /* FOR HEAD */
      else if ((strncmp(request->http_method, "HEAD", 4) == 0)) {
      }

      /* SEND */
      if (send(client_sock, buf, readret, 0) != readret) {
        close_socket(client_sock);
        close_socket(sock);
        fprintf(stderr, "Error sending to client.\n");
        return EXIT_FAILURE;
      }
      memset(buf, 0, BUF_SIZE);

      /* free memory */
      if (request != NULL) {
        free(request->headers);
        free(request);
      }
    }

    /* ERROR Reading */
    if (readret == -1) {
      close_socket(client_sock);
      close_socket(sock);
      fprintf(stderr, "Error reading from client socket.\n");
      return EXIT_FAILURE;
    }

    /* ERROR CLOSING SOCKET */
    if (close_socket(client_sock)) {
      close_socket(sock);
      fprintf(stderr, "Error closing client socket.\n");
      return EXIT_FAILURE;
    }
  }

  close_socket(sock);

  return EXIT_SUCCESS;
}
