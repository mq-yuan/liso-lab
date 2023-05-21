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

int send_message(int sock, int client_sock, const char *buf) {
  ssize_t readret = strlen(buf);
  if (send(client_sock, buf, readret, 0) != readret) {
    close_socket(client_sock);
    close_socket(sock);
    fprintf(stderr, "Error sending to client.\n");
    return 0; // Bad
  }
  return 1; // Normal
}

int send_message_bit(int sock, int client_sock, const char *buf,
                     ssize_t readret) {
  if (send(client_sock, buf, readret, 0) != readret) {
    close_socket(client_sock);
    close_socket(sock);
    fprintf(stderr, "Error sending to client.\n");
    return 0; // Bad
  }
  return 1; // Normal
}

int handle_get_request(int client_sock, const char *filename, char *buf) {
  const char *status = "HTTP/1.1 200 OK\r\n";
  if (strncmp(buf, status, strlen(status)) != 0) {
    send_message(sock, client_sock, buf);
  }
  ssize_t readret = strlen(buf);
  if (send_message(sock, client_sock, buf) == 0) {
    return 0;
  }
  FILE *file = fopen(filename, "rb");
  while (((readret = fread(buf, 1, BUF_SIZE, file)))) {
    if (readret <= 0) {
      break;
    }
    if (feof(file)) {
      printf("Received end of file\n");
    } else if (ferror(file)) {
      printf("Error reading file\n");
    }
    if (send_message_bit(sock, client_sock, buf, readret) == 0) {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[]) {
  int echo_port;
  ssize_t readret;
  socklen_t cli_size;
  struct sockaddr_in addr, cli_addr;
  char buf[BUF_SIZE];
  char fullpath[4096];

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
        response_400(buf, sizeof(buf), &readret);
      }
      /* For HTTP VERSION */
      else if (strncmp(request->http_version, "HTTP/1.1", 9) != 0) {
        response_505(buf, sizeof(buf), &readret);
      }
      /* FOR NO IMPLEMENT */
      else if ((strncmp(request->http_method, "GET", 4) != 0) &&
               (strncmp(request->http_method, "POST", 5) != 0) &&
               (strncmp(request->http_method, "HEAD", 5) != 0)) {
        response_501(buf, sizeof(buf), &readret);
      }
      /* FOR POST */
      else if ((strncmp(request->http_method, "POST", 5) == 0)) {
      }
      /* FOR HEAD */
      else if ((strncmp(request->http_method, "HEAD", 5) == 0)) {
        response_head(fullpath, sizeof(fullpath), request, buf, sizeof(buf),
                      &readret);
      }
      /* FOR GET */
      else if ((strncmp(request->http_method, "GET", 4) == 0)) {
        response_get(fullpath, sizeof(fullpath), request, buf, sizeof(buf),
                     &readret);
      }

      /* SEND */
      if ((strncmp(request->http_method, "GET", 4) != 0)) {
        if (send_message(sock, client_sock, buf) == 0) {
          return EXIT_FAILURE;
        }
      } else {
        if (handle_get_request(client_sock, fullpath, buf) == 0) {
          return EXIT_FAILURE;
        }
      }

      memset(buf, 0, BUF_SIZE);
      memset(fullpath, 0, sizeof(fullpath));

      /* free memory */
      if (request != NULL) {
        free(request->headers);
        free(request);
      }
    }

    /* ERROR Reading */
    if (readret == -1) {
      perror("ERROR readret = -1");
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
