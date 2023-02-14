#include "ptyspawn.h"

void print_help() {
  fprintf(stdout,
          "Tool for executing a command in a new PTY (pseudo-terminal) with new PGID and SID\n"
          "%s\n"
          "      -h     Print this help\n"
          "      -V     Verbose\n"
          "      -v     Version\n",
          PTYSPAWN_USAGE
  );
}

void print_version() {
  fprintf(stdout,
          "ptyspawn %s by VHSgunzo <vhsgunzo.github.io>\n"
          "https://github.com/VHSgunzo/ptyspawn\n",
          PTYSPAWN_VERSION
  );
}

void setup_raw(struct termios *save) {
  struct termios set;
  if (tcgetattr(0, save) < 0) {
      fprintf(stderr, "Unable to read terminal attributes: %m");
      return;
  }
  set = *save;
  cfmakeraw(&set);
  if (tcsetattr(0, TCSANOW, &set) < 0)
      err_sys("Unable to set terminal attributes: %m");
}
void resize_pty(int pty) {
  struct winsize sz;
  if (ioctl(0, TIOCGWINSZ, &sz) < 0) {
    // provide fake size to workaround some problems
    struct winsize defaultsize = {30, 80, 640, 480};
    if (ioctl(pty, TIOCSWINSZ, &defaultsize) < 0) {
      fprintf(stderr, "Cannot set terminal size\n");
    }
    return;
  }
  ioctl(pty, TIOCSWINSZ, &sz);
}

int writeall(int fd, const void *buf, ssize_t count) {
  ssize_t rv;
  while (count > 0) {
    rv = write(fd, buf, count);
    if (rv < 0) {
      if (errno == EINTR)
        continue;
      return rv;
    }
    count -= rv;
    buf += rv;
  }
  return 0;
}

volatile sig_atomic_t winch_happened = 0;

void do_winch(int signal) {
  winch_happened = 1;
}

void do_proxy(int pty) {
  char buf[4096];
  ssize_t count;
  fd_set set;
  sigset_t mask;
  sigset_t select_mask;
  struct sigaction sa;

  // Block WINCH while we're outside the select, but unblock it
  // while we're inside:
  sigemptyset(&mask);
  sigaddset(&mask, SIGWINCH);
  if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
    fprintf(stderr, "sigprocmask: %m");
    return;
  }
  sa.sa_handler = do_winch;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGWINCH, &sa, NULL);
  resize_pty(pty);

  while (1) {
    if (winch_happened) {
      winch_happened = 0;
      resize_pty(pty);
    }
    FD_ZERO(&set);
    FD_SET(0, &set);
    FD_SET(pty, &set);
    sigemptyset(&select_mask);
    if (pselect(pty + 1, &set, NULL, NULL, NULL, &select_mask) < 0) {
      if (errno == EINTR)
        continue;
      fprintf(stderr, "select: %m");
      return;
    }
    if (FD_ISSET(0, &set)) {
      count = read(0, buf, sizeof buf);
      if (count < 0)
        return;
      writeall(pty, buf, count);
    }
    if (FD_ISSET(pty, &set)) {
      count = read(pty, buf, sizeof buf);
      if (count <= 0)
        return;
      writeall(1, buf, count);
    }
  }
}

int get_pt() {
  return posix_openpt(O_RDWR | O_NOCTTY);
}

int main(int argc, char * argv[]) {
  int pty, c, verbose, returncode, status;
  pid_t pid;
  struct termios saved_termios;

  verbose = 0;
  returncode = 1; /* return exec code by default */

  opterr = 0; /* don't want getopt() writing to stderr */
  while ((c = getopt(argc, argv, OPTSTR)) != EOF) {
    switch (c) {

    case 'V':
      /* verbose */
      verbose = 1;
      break;

    case 'v':
      /* version */
      print_version();
      exit(0);

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
    err_quit("%s", PTYSPAWN_USAGE);

  if ((pty = get_pt()) < 0)
    err_sys("Unable to allocate a new pseudo-terminal: %m");
  if (unlockpt(pty) < 0)
    err_sys("Unable to unlockpt: %m");
  if (grantpt(pty) < 0)
    err_sys("Unable to grantpt: %m");
  char * slave_name = ptsname(pty);

  pid = fork();
  if (pid < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    /* child */
    fflush(stdout);

    int fds;
    setpgid(0, getppid());
    setsid();
    fds = open(slave_name, O_RDONLY, 0);
    if (dup2(fds, STDIN_FILENO) != STDIN_FILENO)
      err_sys("dup2 error to stdin");
    close(fds);
    fds = open(slave_name, O_WRONLY, 0);
    if (dup2(fds, STDOUT_FILENO) != STDOUT_FILENO)
      err_sys("dup2 error to stdout");
    if (dup2(fds, STDERR_FILENO) != STDERR_FILENO)
      err_sys("dup2 error to stderr");
    close(fds);
    close(pty);

    setenv("PTYSPAWN_PTY", slave_name, 1);
    int self_ppid = getppid();
    char * ptyspawn_pid = malloc(sizeof(self_ppid));
    sprintf(ptyspawn_pid, "%d", self_ppid);
    setenv("PTYSPAWN_PID", ptyspawn_pid, 1);

    if (verbose) {
      fprintf(stderr, "[ENV]: PTYSPAWN_PTY = %s\n", slave_name);
      fprintf(stderr, "[ENV]: PTYSPAWN_PID = %s\n", ptyspawn_pid);
    };

    if (execvp(argv[optind], & argv[optind]) < 0)
      err_sys("can't execute: %s", argv[optind]);
  };

  setup_raw(&saved_termios);
  do_proxy(pty);
  do {
      errno = 0;
      if (tcsetattr(0, TCSANOW, &saved_termios) && errno != EINTR)
          err_sys("Unable to tcsetattr: %m");
  } while (errno == EINTR);

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
