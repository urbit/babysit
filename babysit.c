#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

typedef long long msec_t;

#include "config.h"

static msec_t* times;

msec_t
get_msecs()
{
  struct timeval tm;
  gettimeofday(&tm, NULL);
  return tm.tv_sec * 1000 + tm.tv_usec / 1000;
}

void
babysit(const char* myname, const char* file, char* const argv[], int i)
{
  msec_t now = get_msecs();

  assert(now > flapping_ms);
  if ( now - times[i % flappingrestarts] < flapping_ms ) {
    fprintf(stderr, "[%s] %s flapping - %d restarts in %lldms\n",
            myname, file, flappingrestarts, now - times[i % flappingrestarts]);
    exit(-1);
  }
  times[i % flappingrestarts] = now;
  fprintf(stderr, "[%s] Starting %s (%d)\n", myname, file, i);
  {
    pid_t pid = fork();
    if ( pid == 0 ) {
      if ( -1 == execvp(file, argv) ) {
        perror("execvp");
        exit(-1);
      }
    }
    else if ( pid > 0 ) {
      int stat;
      if ( -1 == waitpid(pid, &stat, 0) ) {
        if ( errno == ECHILD ) {
          babysit(myname, file, argv, i + 1);
        }
        else {
          perror("waitpid");
          exit(-1);
        }
      }
      else {
        babysit(myname, file, argv, i + 1);
      }
    }
    else {
      perror("fork");
      exit(-1);
    }
  }
}

void
usage(const char* myname)
{
  fprintf(stderr, "usage: %s <file> [args]\n", myname);
}

int
main(int argc, char *argv[])
{
  char* path;
  char** args;

  if ( argc < 2 ) {
    usage(argv[0]);
    exit(1);
  }
  path = argv[1];
  args = malloc(sizeof(char*) * argc);
  {
    int i;
    for ( i = 0; i < argc - 1; i++ ) {
      args[i] = argv[i + 1];
    }
  }
  args[argc - 1] = NULL;

  times = calloc(flappingrestarts, sizeof(msec_t));

  babysit(argv[0], path, args, 0);

  free(args);
  free(times);
  return 0;
}
