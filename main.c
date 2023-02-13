#include "ptyspawn.h"
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef LINUX
#define OPTSTR "+d:einvrh"
#else
#define OPTSTR "d:einvrh"
#endif

static void set_noecho(int); /* at the end of this file */
void do_driver(char * ); /* in the file driver.c */
void loop(int, int); /* in the file loop.c */

void
print_help() {
  fprintf(stdout,
          "Tool for executing a command in a new PTY (pseudo-terminal) v0.0.1\n"
          "   Usage: ptyspawn [ -d driver -einvrh ] command {command args}\n"
          "         -h     Print this help\n"
          "         -d     Set driver for stdin/stdout\n"
          "         -e     Noecho for slave pty's line discipline\n"
          "         -i     Ignore EOF on standard input\n"
          "         -n     Not interactive\n"
          "         -v     Verbose\n"
          "         -r     Return exec code (enabled by default)\n"
  );
}

int
main(int argc, char * argv[]) {
  int fdm, c, ignoreeof, interactive, noecho, verbose, returncode, status;
  pid_t pid;
  char * driver;
  char slave_name[20];
  struct termios orig_termios;
  struct winsize size;

  interactive = isatty(STDIN_FILENO);
  ignoreeof = 0;
  noecho = 0;
  verbose = 0;
  driver = NULL;
  returncode = 1; /* return exec code by default */

  opterr = 0; /* don't want getopt() writing to stderr */
  while ((c = getopt(argc, argv, OPTSTR)) != EOF) {
    switch (c) {
    case 'd':
      /* driver for stdin/stdout */
      driver = optarg;
      break;

    case 'e':
      /* noecho for slave pty's line discipline */
      noecho = 1;
      break;

    case 'i':
      /* ignore EOF on standard input */
      ignoreeof = 1;
      break;

    case 'n':
      /* not interactive */
      interactive = 0;
      break;

    case 'v':
      /* verbose */
      verbose = 1;
      break;

    case 'r':
      /* return exec code */
      returncode = 1;
      break;

    case 'h':
      /* print help */
      print_help();
      exit(0);
      break;

    case '?':
      err_quit("unrecognized option: -%c", optopt);
    }
  }
  if (optind >= argc)
    err_quit("Usage: ptyspawn [ -d driver -einvrh ] command {command args}");

  if (interactive) {
    /* fetch current termios and window size */
    if (tcgetattr(STDIN_FILENO, & orig_termios) < 0)
      err_sys("tcgetattr error on stdin");
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char * ) & size) < 0)
      err_sys("TIOCGWINSZ error");
    pid = pty_fork( & fdm, slave_name, sizeof(slave_name), &
      orig_termios, & size);
  } else {
    pid = pty_fork( & fdm, slave_name, sizeof(slave_name), NULL, NULL);
  }

  if (pid < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    /* child */
    if (noecho)
      set_noecho(STDIN_FILENO); /* stdin is slave pty */

    if (execvp(argv[optind], & argv[optind]) < 0)
      err_sys("can't execute: %s", argv[optind]);
  }

  if (verbose) {
    fprintf(stderr, "slave name = %s\n", slave_name);
    if (driver != NULL)
      fprintf(stderr, "driver = %s\n", driver);
  }

  if (interactive && driver == NULL) {
    if (tty_raw(STDIN_FILENO) < 0) /* user's tty to raw mode */
      err_sys("tty_raw error");
    if (atexit(tty_atexit) < 0) /* reset user's tty on exit */
      err_sys("atexit error");
  }

  if (driver)
    do_driver(driver); /* changes our stdin/stdout */

  loop(fdm, ignoreeof); /* copies stdin -> ptym, ptym -> stdout */

  if (returncode) {
    if (waitpid(pid, & status, 0) < 0)
      err_sys("waitpid error");
    if (WIFEXITED(status))
      exit(WEXITSTATUS(status));
    else
      exit(EXIT_FAILURE);
  }

  exit(0);
}

static void
set_noecho(int fd) /* turn off echo (for slave pty) */ {
  struct termios stermios;

  if (tcgetattr(fd, & stermios) < 0)
    err_sys("tcgetattr error");

  stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

  /*
   * Also turn off NL to CR/NL mapping on output.
   */
  stermios.c_oflag &= ~(ONLCR);

  if (tcsetattr(fd, TCSANOW, & stermios) < 0)
    err_sys("tcsetattr error");
}
