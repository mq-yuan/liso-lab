#include "cgi.h"
#include <stdlib.h>

CGI_param *build_cgi_param(Request *request, const char *fullpath,
                           host_and_port hap) {
  CGI_param *param = (CGI_param *)malloc(sizeof(CGI_param));
  if (param == NULL) {
    errorLOG("[FATAL] Fails to allocate memory for CGI parametes");
  }

  param->filename = newstring(fullpath);
  param->args[0] = newstring(fullpath);
  param->args[1] = NULL;

  int index = 0;
  /* envp taken from str */
  set_envp_field_by_str("GATEWAY_INTERFACE", "CGI/1.1", param, index++);
  set_envp_field_by_str("SERVER_SOFTWARE", "Liso/1.0", param, index++);
  set_envp_field_by_str("SERVER_PROTOCOL", "HTTP/1.1", param, index++);
  set_envp_field_by_str("REQUEST_METHOD", request->http_method, param, index++);
  set_envp_field_by_str("REQUEST_URI", request->http_uri, param, index++);
  set_envp_field_by_str("SCRIPT_NAME", "/cgi", param, index++);
  set_envp_field_by_str("PATH_INFO", "", param, index++);
  set_envp_field_by_str("REMOTE_ADDR", hap.host, param, index++);

  char port_str[8];
  memset(port_str, 0, 8);
  sprintf(port_str, "%d", hap.port);
  set_envp_field_by_str("SERVER_PORT", port_str, param, index++);

  char *query_string;
  int split = strpos(request->http_uri, '?');
  if (split != -1) {
    query_string = newstring(request->http_uri + split);
  } else {
    query_string = "";
  }
  set_envp_field_by_str("QUERY_STRING", query_string, param, index++);

  /* envp taken from request */
  set_envp_field_by_header(request, "Content-Length", "CONTENT_LENGTH", param,
                           index++);
  set_envp_field_by_header(request, "Content-Type", "CONTENT_TYPE", param,
                           index++);
  set_envp_field_by_header(request, "Accept", "HTTP_ACCEPT", param, index++);
  set_envp_field_by_header(request, "Referer", "HTTP_REFERER", param, index++);
  set_envp_field_by_header(request, "Accept-Encoding", "HTTP_ACCEPT_ENCODING",
                           param, index++);
  set_envp_field_by_header(request, "Accept-Language", "HTTP_ACCEPT_LANGUAGE",
                           param, index++);
  set_envp_field_by_header(request, "Accept-Charset", "HTTP_ACCEPT_CHARSET",
                           param, index++);
  set_envp_field_by_header(request, "Cookie", "HTTP_COOKIE", param, index++);
  set_envp_field_by_header(request, "User-Agent", "HTTP_USER_AGENT", param,
                           index++);
  set_envp_field_by_header(request, "Host", "HTTP_HOST", param, index++);
  set_envp_field_by_header(request, "Connection", "HTTP_CONNECTION", param,
                           index++);

  param->envp[index++] = NULL;

  return param;
}

void free_CGI_param(CGI_param *param) {
  int i = 0;
  while (param->envp[i] != NULL) {
    free(param->envp[i]);
    i++;
  }
  free(param);
}

void set_envp_field_by_str(const char *envp_name, const char *value,
                           CGI_param *param, int index) {
  char buf[CGI_ENVP_INFO_MAXLEN];
  memset(buf, 0, CGI_ENVP_INFO_MAXLEN);
  sprintf(buf, "%s=%s", envp_name, value);
  param->envp[index] = newstring(buf);
}

void set_envp_field_by_header(Request *request, const char *header_name,
                              const char *envp_name, CGI_param *param,
                              int index) {
  char *value = get_header_value(request, header_name);
  set_envp_field_by_str(envp_name, value, param, index);
}

int handle_cgi(CGI_param *cgi_param, char *buf) {
  /*************** BEGIN VARIABLE DECLARATIONS **************/
  CGI_executor *executor = NULL;
  executor = (CGI_executor *)malloc(sizeof(CGI_executor));
  executor->cgi_parameter = cgi_param;
  /*************** END VARIABLE DECLARATIONS **************/

  /*************** BEGIN PIPE **************/
  /* 0 can be read from, 1 can be written to */
  if (pipe(executor->stdin_pipe) < 0) {
    fprintf(stderr, "Error piping for stdin.\n");
    return EXIT_FAILURE;
  }

  if (pipe(executor->stdout_pipe) < 0) {
    fprintf(stderr, "Error piping for stdout.\n");
    return EXIT_FAILURE;
  }
  /*************** END PIPE **************/

  /*************** BEGIN FORK **************/
  executor->pid = fork();
  /* not good */
  if (executor->pid < 0) {
    fprintf(stderr, "Something really bad happened when fork()ing.\n");
    return EXIT_FAILURE;
  }

  /* child, setup environment, execve */
  if (executor->pid == 0) {
    /*************** BEGIN EXECVE ****************/
    close(executor->stdout_pipe[0]);
    close(executor->stdin_pipe[1]);
    dup2(executor->stdout_pipe[1], fileno(stdout));
    dup2(executor->stdin_pipe[0], fileno(stdin));
    /* you should probably do something with stderr */

    /* pretty much no matter what, if it returns bad things happened... */
    if (execve(cgi_param->filename, cgi_param->args, cgi_param->envp)) {
      execve_error_handler();
      fprintf(stderr, "Error executing execve syscall.\n");
      return EXIT_FAILURE;
    }
    /*************** END EXECVE ****************/
  }

  if (executor->pid > 0) {
    int readret;
    int status;
    waitpid(executor->pid, &status, 0);

    close(executor->stdout_pipe[1]);
    close(executor->stdin_pipe[0]);
    close(executor->stdin_pipe[1]); /* finished writing to spawn */
    while ((readret = read(executor->stdout_pipe[0], buf, BUF_SIZE - 1)) > 0) {
      buf[readret] = '\0'; /* nul-terminate string */
    }

    close(executor->stdout_pipe[0]);
    close(executor->stdin_pipe[1]);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      memset(buf, 0, BUF_SIZE);
      strcat(buf, "");
    }

    if (readret == 0) {
      return EXIT_SUCCESS;
    }
  }
  return 1;
}

/**************** BEGIN UTILITY FUNCTIONS ***************/
/* error messages stolen from: http://linux.die.net/man/2/execve */
void execve_error_handler() {
  switch (errno) {
  case E2BIG:
    fprintf(stderr, "The total number of bytes in the environment \
(envp) and argument list (argv) is too large.\n");
    return;
  case EACCES:
    fprintf(stderr, "Execute permission is denied for the file or a \
script or ELF interpreter.\n");
    return;
  case EFAULT:
    fprintf(stderr, "filename points outside your accessible address \
space.\n");
    return;
  case EINVAL:
    fprintf(stderr, "An ELF executable had more than one PT_INTERP \
segment (i.e., tried to name more than one \
interpreter).\n");
    return;
  case EIO:
    fprintf(stderr, "An I/O error occurred.\n");
    return;
  case EISDIR:
    fprintf(stderr, "An ELF interpreter was a directory.\n");
    return;
  case ELIBBAD:
    fprintf(stderr, "An ELF interpreter was not in a recognised \
format.\n");
    return;
  case ELOOP:
    fprintf(stderr, "Too many symbolic links were encountered in \
resolving filename or the name of a script \
or ELF interpreter.\n");
    return;
  case EMFILE:
    fprintf(stderr, "The process has the maximum number of files \
open.\n");
    return;
  case ENAMETOOLONG:
    fprintf(stderr, "filename is too long.\n");
    return;
  case ENFILE:
    fprintf(stderr, "The system limit on the total number of open \
files has been reached.\n");
    return;
  case ENOENT:
    fprintf(stderr, "The file filename or a script or ELF interpreter \
does not exist, or a shared library needed for \
file or interpreter cannot be found.\n");
    return;
  case ENOEXEC:
    fprintf(stderr, "An executable is not in a recognised format, is \
for the wrong architecture, or has some other \
format error that means it cannot be \
executed.\n");
    return;
  case ENOMEM:
    fprintf(stderr, "Insufficient kernel memory was available.\n");
    return;
  case ENOTDIR:
    fprintf(stderr, "A component of the path prefix of filename or a \
script or ELF interpreter is not a directory.\n");
    return;
  case EPERM:
    fprintf(stderr, "The file system is mounted nosuid, the user is \
not the superuser, and the file has an SUID or \
SGID bit set.\n");
    return;
  case ETXTBSY:
    fprintf(stderr, "Executable was open for writing by one or more \
processes.\n");
    return;
  default:
    fprintf(stderr, "Unkown error occurred with execve().\n");
    return;
  }
}
