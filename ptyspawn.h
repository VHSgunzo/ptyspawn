#ifndef	_PTYSPAWN_H
#define	_PTYSPAWN_H

#define PTYSPAWN_VERSION "v0.0.3"
#define PTYSPAWN_USAGE "Usage: ptyspawn [ -vhV ] command {command args}"

#define _POSIX_C_SOURCE 200809L

#if defined(SOLARIS)		/* Solaris 10 */
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 700
#endif
#define OPTSTR "vhV"

#include <sys/types.h>		/* some systems still require this */
#include <sys/stat.h>
#include <termios.h>	/* for winsize */
#if defined(MACOS) || !defined(TIOCGWINSZ)
#include <sys/ioctl.h>
#endif

#include <stdio.h>		/* for convenience */
#include <stdlib.h>		/* for convenience */
#include <stddef.h>		/* for offsetof */
#include <string.h>		/* for convenience */
#include <unistd.h>		/* for convenience */
#include <signal.h>		/* for SIG_ERR */
#include <errno.h>		/* for definition of errno */
#include <stdarg.h>		/* ISO C variable aruments */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>

#define	MAXLINE	4096		/* max line length */

/*
 * Default file access permissions for new files.
 */
#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
 * Default permissions for new directories.
 */
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

typedef void Sigfunc (int);	/* for signal handlers */

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

void err_msg (const char *, ...);	/* {App misc_source} */
void err_dump (const char *, ...) __attribute__ ((noreturn));
void err_quit (const char *, ...) __attribute__ ((noreturn));
void err_cont (int, const char *, ...);
void err_exit (int, const char *, ...) __attribute__ ((noreturn));
void err_ret (const char *, ...);
void err_sys (const char *, ...) __attribute__ ((noreturn));
void cfmakeraw(struct termios *);

#endif
