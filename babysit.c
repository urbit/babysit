#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"

static uint64_t* times;
static uint32_t restarts;
static uint64_t interval;
static char* argv0;
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
vlog(uint64_t now, const char* fmt, va_list ap)
{
  fprintf(stderr, "%14llu [%s]: ", now, argv0);
  vfprintf(stderr, fmt, ap);
}

static void
log(uint64_t now, const char* fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vlog(now, fmt, ap);
  va_end(ap);
}

static void
babysit(uint32_t i)
{
  uint64_t now = get_msecs();
  assert(now > interval * 1000L);
  i = i % restarts;
  if ( now - times[i % restarts] < interval * 1000L ) {
    log(now, "%s flapping - %d restarts in %lums\n",
             file, restarts, now - times[i]);
    exit(2);
  }
  times[i] = now;
  log(now, "Starting %s (%d)\n", file, i);

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
  char *optarg;
  char flag;

  argv0 = *argv++;
  if ( argc < 2 ) {
    usage();
  }
  argc--;

  restarts = crestarts;
  interval = cinterval;

  while ( argv[0][0] == '-' && argc > 1 ) {
    if ( strlen(argv[0]) != 2 ) {
      usage();
    }
    flag = argv[0][1];
    optarg = argv[1];

    argv += 2;
    argc -= 2;

    switch ( flag ) {
      default: {
        fprintf(stderr, "Unknown option %c\n", flag);
        usage();
      }
      case 'r': {
        restarts = atol(optarg);
        break;
      }
      case 'i': {
        interval = atol(optarg);
      }
    }
    fprintf(stderr, "argc:%d argv:%s\n", argc, *argv);
  }

  if ( restarts < 1 || interval < 1 || argc < 1 ) {
    usage();
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
