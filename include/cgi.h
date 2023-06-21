#include "constants.h"
#include "log.h"
#include "parse.h"
#include "utils.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct CGI_param {
  char *filename;
  char *args[CGI_ARGS_LEN];
  char *envp[CGI_ENVP_LEN];
} CGI_param;

typedef struct CGI_executor {
  int pid;

  /* { write data --> stdin_pipe[1] } -> { stdin_pipe[0] -->stdin } */
  int stdin_pipe[2];

  /* { read data <--  stdout_pipe[0] } <-- {stdout_pipe[1]<-- stdout } */
  int stdout_pipe[2];

  char cgi_buffer[BUF_SIZE];
  int readret;

  CGI_param *cgi_parameter;
} CGI_executor;

CGI_param *build_cgi_param(Request *request, const char *fullpath,
                           host_and_port hap);
void free_CGI_param(CGI_param *param);
void set_envp_field_by_str(const char *envp_name, const char *value,
                           CGI_param *param, int index);
void set_envp_field_by_header(Request *request, const char *header_name,
                              const char *envp_name, CGI_param *param,
                              int index);
int handle_cgi(CGI_param *cgi_param, char *buf);
void execve_error_handler();
