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

#include "constants.h"
#include "parse.h"
#include "response_parse.h"
#include "utils.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int sock, client_sock;

int close_socket(int sock) {
  if (close(sock)) {
    fprintf(stderr, "Failed closing socket.\n");
    errorLOG("Failed closing socket.\r\n");
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
    errorLOG("Error sending to client.\r\n");
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
    errorLOG("Error sending to client.\r\n");
    return 0; // Bad
  }
  return 1; // Normal
}

int handle_get_request(int client_sock, const char *filename, char *buf) {
  const char *status = "HTTP/1.1 200 OK\r\n";
  if (strncmp(buf, status, strlen(status)) != 0) {
    send_message(sock, client_sock, buf);
    return 1;
  }
  ssize_t readret = strlen(buf);
  ssize_t _readret;
  FILE *file = fopen(filename, "rb");
  while ((_readret = fread(buf + readret, 1, BUF_SIZE - readret, file)) &&
         (_readret > 0)) {
    if (send_message_bit(sock, client_sock, buf, readret + _readret) == 0) {
      return 0;
    }
    /* clear the buf */
    memset(buf, 0, readret + _readret);
    readret = 0; // clear the response head
  }
  return 1;
}

int main(int argc, char *argv[]) {
  int echo_port;
  ssize_t readret;
  socklen_t cli_size;
  struct sockaddr_in cli_addr;
  struct sockaddr_in addr[1024];
  char buf[BUF_SIZE];
  char fullpath[BUF_SIZE];
  char request_regions[MAX_REGIONS][BUF_SIZE];
  ssize_t base;
  int max_fd;
  int activity;
  fd_set readfds;

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
    errorLOG("Failed creating socket.\r\n");
    return EXIT_FAILURE;
  }

  cli_addr.sin_family = AF_INET;
  cli_addr.sin_port = htons(echo_port);
  cli_addr.sin_addr.s_addr = INADDR_ANY;

  addr[sock] = cli_addr;
  FD_ZERO(&readfds);
  FD_SET(sock, &readfds);
  max_fd = sock;

  /* Allow address reuse */
  int reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
    perror("Set socket option failed");
    exit(EXIT_FAILURE);
  }

  /* servers bind sockets to ports---notify the OS they accept connections */
  if (bind(sock, (struct sockaddr *)&addr[sock], sizeof(addr[sock]))) {
    releasePort(echo_port);
    if (bind(sock, (struct sockaddr *)&addr[sock], sizeof(addr[sock]))) {
      close_socket(sock);
      perror("Bind socket");
      fprintf(stderr, "Failed binding socket.\n");
      errorLOG("Failed binding socket.\r\n");
      return EXIT_FAILURE;
    }
  }

  if (listen(sock, 5)) {
    close_socket(sock);
    fprintf(stderr, "Error listening on socket.\n");
    errorLOG("Error listening on socket.\r\n");
    return EXIT_FAILURE;
  }

  /* finally, loop waiting for input and then write it back */
  while (1) {
    fd_set readset = readfds;
    activity = select(FD_SETSIZE, &readset, NULL, NULL, NULL);
    if ((activity < 0)) {
      perror("Select error");
    }
    if (FD_ISSET(sock, &readset)) {
      int newSocket;
      cli_size = sizeof(cli_addr);
      if ((newSocket = accept(sock, (struct sockaddr *)&cli_addr,
                              (socklen_t *)&cli_size)) < 0) {
        perror("Accept error");
        exit(EXIT_FAILURE);
      }
      addr[newSocket] = cli_addr;
      if (newSocket >= FD_SETSIZE) {
        close(newSocket);
        return EXIT_FAILURE;
      }
      FD_SET(newSocket, &readfds);
      if (newSocket > max_fd) {
        max_fd = newSocket;
      }
      if (--activity == 0)
        continue;
    }

    for (int i = sock + 1; i <= max_fd; i++) {
      if (FD_ISSET(i, &readfds)) {

        readret = 0;
        base = 0;
        client_sock = i;

        while ((readret = recv(client_sock, buf + base, BUF_SIZE - base, 0)) >=
               1) {
          /* split the buf */
          int regions_num = char_split(buf, "\r\n\r\n", request_regions);

          /* mark whether last region is completed */

          for (int _r = 0; _r < regions_num - 1; _r++) {
            char *token = request_regions[_r];
            /* Parse */
            size_t n = (BUF_SIZE > strlen(token)) ? strlen(token) : BUFSIZ;
            Request *request = parse(token, n, client_sock);

            /* Generating the Response Messages for Unimplemented and Formatting
             * Errors*/
            /* For NULL */
            if (request == NULL) {
              response_400(token, strlen(token), &readret);
            }
            /* For HTTP VERSION */
            else if (strncmp(request->http_version, "HTTP/1.1", 9) != 0) {
              response_505(token, strlen(token), &readret);
            }
            /* FOR NO IMPLEMENT */
            else if ((strncmp(request->http_method, "GET", 4) != 0) &&
                     (strncmp(request->http_method, "POST", 5) != 0) &&
                     (strncmp(request->http_method, "HEAD", 5) != 0)) {
              response_501(token, strlen(token), &readret);
            }
            /* FOR POST */
            else if ((strncmp(request->http_method, "POST", 5) == 0)) {
              char _token[LOG_BUF_SIZE];
              sprintf(_token, " \"%s %s %s\" 200 ", request->http_method,
                      request->http_uri, request->http_version);
              accessLOG(_token);
              memset(_token, 0, sizeof(_token));
            }
            /* FOR HEAD */
            else if ((strncmp(request->http_method, "HEAD", 5) == 0)) {
              response_head(fullpath, sizeof(fullpath), request, token,
                            strlen(token), &readret);
            }
            /* FOR GET */
            else if ((strncmp(request->http_method, "GET", 4) == 0)) {
              response_get(fullpath, sizeof(fullpath), request, token,
                           strlen(token), &readret);
            }

            /* SEND */
            if ((request == NULL) ||
                (strncmp(request->http_method, "GET", 4) != 0)) {
              if (send_message(sock, client_sock, token) == 0) {
                return EXIT_FAILURE;
              }
            } else {
              if (handle_get_request(client_sock, fullpath, token) == 0) {
                return EXIT_FAILURE;
              }
            }

            memset(token, 0, BUF_SIZE);
            memset(fullpath, 0, sizeof(fullpath));

            /* free memory */
            if (request != NULL) {
              free(request->headers);
              free(request);
            }
          }

          memset(buf, 0, BUF_SIZE);

          strcat(buf, request_regions[regions_num - 1]);
          base = strlen(buf);
          if (base == 0)
            break;
        }

        /* ERROR Reading */
        if (readret == -1) {
          close_socket(client_sock);
          close_socket(sock);
          fprintf(stderr, "Error reading from client socket.\n");
          errorLOG("Error reading from client socket.\r\n");
          return EXIT_FAILURE;
        }

        /* ERROR CLOSING SOCKET */
        if (close_socket(client_sock)) {
          close_socket(sock);
          fprintf(stderr, "Error closing client socket.\n");
          errorLOG("Error closing client socket.\r\n");
          return EXIT_FAILURE;
        }
      }
      if (--activity == 0)
        break;
    }
  }

  close_socket(sock);

  return EXIT_SUCCESS;
}
