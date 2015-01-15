#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "arg.h"

#include "config.h"

char* argv0;

static uint64_t* times;
static uint32_t restarts;
static uint64_t interval;
static char* file;
static char** args;

static void
usage()
{
  fprintf(stderr, "usage: %s [opts] <file> [file-args]\n"
                  "opts:\n"
                  "-r <restarts> restarts per interval [default: %u]\n"
                  "-i <interval> flapping interval [default: %u]\n",
                  argv0, crestarts, cinterval);
  exit(1);
}

static uint64_t
get_msecs()
{
  struct timeval tm;

  gettimeofday(&tm, NULL);
  return tm.tv_sec * 1000 + tm.tv_usec / 1000;
}

static void
vblog(uint64_t now, const char* fmt, va_list ap)
{
  char buf[2048];
  int r;

  r = snprintf(buf, 2048, "%14" PRIu64 " [%s]: ", now, argv0);
  assert(r > 0 && r < 2048);
  vsnprintf(buf + r, 2048 - r, fmt, ap);
  fprintf(stderr, "%s\n", buf);
}

static void
blog(uint64_t now, const char* fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vblog(now, fmt, ap);
  va_end(ap);
}

static void
babysit(uint32_t i)
{
  uint64_t now = get_msecs();
  assert(now > interval * 1000L);
  i = i % restarts;
  if ( now - times[i % restarts] < interval * 1000L ) {
    blog(now, "%s flapping - %d restarts in %lums",
              file, restarts, now - times[i]);
    exit(2);
  }
  times[i] = now;
  blog(now, "Starting %s (%d)", file, i);

  {
    pid_t pid = fork();
    if ( pid == 0 ) {
      if ( -1 == execvp(file, args) ) {
        perror("execvp");
        exit(2);
      }
    }
    else if ( pid > 0 ) {
      int stat;
      if ( -1 == waitpid(pid, &stat, 0) ) {
        if ( errno == ECHILD ) {
          babysit(i + 1);
        }
        else {
          perror("waitpid");
          exit(2);
        }
      }
      else {
        babysit(i + 1);
      }
    }
    else {
      perror("fork");
      exit(2);
    }
  }
}

int
main(int argc, char *argv[])
{
  restarts = crestarts;
  interval = cinterval;

  ARGBEGIN {
    case 'i':
      interval = atol(EARGF(usage()));
      break;
    case 'r':
      restarts = atol(EARGF(usage()));
      break;
    default: usage();
  } ARGEND;

  if ( argc < 1 ) {
    fprintf(stderr, "Filename not supplied.\n");
    usage();
  }
  if ( restarts < 1 ) {
    fprintf(stderr, "Giving up after 0 restarts...\n");
    exit(2);
  }

  file = *argv;
  args = malloc(sizeof(char*) * (argc + 1));
  {
    int32_t i;
    for ( i = 0; i < argc; i++ ) {
      args[i] = argv[i];
    }
  }
  args[argc] = NULL;

  times = calloc(restarts, sizeof(uint64_t));
  babysit(0);

  free(args);
  free(times);
  return 0;
}
