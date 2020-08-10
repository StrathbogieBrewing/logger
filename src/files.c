#include <sys/cdefs.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

static int build(char *, mode_t);
static void usage(void);

int vflag;

int main(int argc, char *argv[]) {
  int ch, exitval, success, pflag;
  mode_t omode;
  void *set = NULL;
  char *mode;

  omode = pflag = 0;
  mode = NULL;
  while ((ch = getopt(argc, argv, "m:pv")) != -1)
    switch (ch) {
    case 'm':
      mode = optarg;
      break;
    case 'p':
      pflag = 1;
      break;
    case 'v':
      vflag = 1;
      break;
    case '?':
    default:
      usage();
    }

  argc -= optind;
  argv += optind;
  if (argv[0] == NULL)
    usage();

  if (mode == NULL) {
    omode = S_IRWXU | S_IRWXG | S_IRWXO;
  } else {
    if ((set = setmode(mode)) == NULL)
      errx(1, "invalid file mode: %s", mode);
    omode = getmode(set, S_IRWXU | S_IRWXG | S_IRWXO);
    free(set);
  }

  for (exitval = 0; *argv != NULL; ++argv) {
    success = 1;
    if (pflag) {
      if (build(*argv, omode))
        success = 0;
    } else if (mkdir(*argv, omode) < 0) {
      if (errno == ENOTDIR || errno == ENOENT)
        warn("%s", dirname(*argv));
      else
        warn("%s", *argv);
      success = 0;
    } else if (vflag)
      (void)printf("%s\n", *argv);

    if (!success)
      exitval = 1;
    /*
     * The mkdir() and umask() calls both honor only the low
     * nine bits, so if you try to set a mode including the
     * sticky, setuid, setgid bits you lose them.  Don't do
     * this unless the user has specifically requested a mode,
     * as chmod will (obviously) ignore the umask.
     */
    if (success && mode != NULL && chmod(*argv, omode) == -1) {
      warn("%s", *argv);
      exitval = 1;
    }
  }
  exit(exitval);
}

int build(char *path, mode_t omode) {
  struct stat sb;
  mode_t numask, oumask;
  int first, last, retval;
  char *p;

  p = path;
  oumask = 0;
  retval = 0;
  if (p[0] == '/') /* Skip leading '/'. */
    ++p;
  for (first = 1, last = 0; !last; ++p) {
    if (p[0] == '\0')
      last = 1;
    else if (p[0] != '/')
      continue;
    *p = '\0';
    if (!last && p[1] == '\0')
      last = 1;
    if (first) {
      /*
       * POSIX 1003.2:
       * For each dir operand that does not name an existing
       * directory, effects equivalent to those cased by the
       * following command shall occcur:
       *
       * mkdir -p -m $(umask -S),u+wx $(dirname dir) &&
       *    mkdir [-m mode] dir
       *
       * We change the user's umask and then restore it,
       * instead of doing chmod's.
       */
      oumask = umask(0);
      numask = oumask & ~(S_IWUSR | S_IXUSR);
      (void)umask(numask);
      first = 0;
    }
    if (last)
      (void)umask(oumask);
    if (mkdir(path, last ? omode : S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
      if (errno == EEXIST || errno == EISDIR) {
        if (stat(path, &sb) < 0) {
          warn("%s", path);
          retval = 1;
          break;
        } else if (!S_ISDIR(sb.st_mode)) {
          if (last)
            errno = EEXIST;
          else
            errno = ENOTDIR;
          warn("%s", path);
          retval = 1;
          break;
        }
      } else {
        warn("%s", path);
        retval = 1;
        break;
      }
    } else if (vflag)
      printf("%s\n", path);
    if (!last)
      *p = '/';
  }
  if (!first && !last)
    (void)umask(oumask);
  return (retval);
}

void usage(void) {

  (void)fprintf(stderr, "usage: mkdir [-pv] [-m mode] directory_name ...\n");
  exit(EX_USAGE);
}

// int mkpath(char *path, mode_t mode) {
//   struct stat sb;
//   char *slash;
//   int done;
//
//   slash = path;
//
//   mode_t dir_mode = mode | S_IWUSR | S_IXUSR;
//
//   for (;;) {
//     slash += strspn(slash, "/");
//     slash += strcspn(slash, "/");
//
//     done = (*slash == '\0');
//     *slash = '\0';
//
//     if (mkdir(path, done ? mode : dir_mode) == 0) {
//       if (mode > 0777 && chmod(path, mode) == -1)
//         return (-1);
//     } else {
//       int mkdir_errno = errno;
//
//       if (stat(path, &sb) == -1) {
//         /* Not there; use mkdir()s errno */
//         errno = mkdir_errno;
//         return (-1);
//       }
//       if (!S_ISDIR(sb.st_mode)) {
//         /* Is there, but isn't a directory */
//         errno = ENOTDIR;
//         return (-1);
//       }
//     }
//
//     if (done)
//       break;
//
//     *slash = '/';
//   }
//
//   return (0);
// }
//
// /*	$OpenBSD: mkdir.c,v 1.31 2019/06/28 13:34:59 deraadt Exp $	*/
// /*	$NetBSD: mkdir.c,v 1.14 1995/06/25 21:59:21 mycroft Exp $	*/
//
// /*
//  * Copyright (c) 1983, 1992, 1993
//  *	The Regents of the University of California.  All rights reserved.
//  *
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions
//  * are met:
//  * 1. Redistributions of source code must retain the above copyright
//  *    notice, this list of conditions and the following disclaimer.
//  * 2. Redistributions in binary form must reproduce the above copyright
//  *    notice, this list of conditions and the following disclaimer in the
//  *    documentation and/or other materials provided with the distribution.
//  * 3. Neither the name of the University nor the names of its contributors
//  *    may be used to endorse or promote products derived from this software
//  *    without specific prior written permission.
//  *
//  * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
//  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
//  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
//  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
//  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
//  * SUCH DAMAGE.
//  */
//
// #include <sys/stat.h>
// #include <sys/types.h>
//
// #include <err.h>
// #include <errno.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
//
// extern char *__progname;
//
// int mkpath(char *, mode_t, mode_t);
// static void __dead usage(void);
//
// int main(int argc, char *argv[]) {
//   int ch, rv, exitval, pflag;
//   void *set;
//   mode_t mode, dir_mode;
//
//   /*
//    * The default file mode is a=rwx (0777) with selected permissions
//    * removed in accordance with the file mode creation mask.  For
//    * intermediate path name components, the mode is the default modified
//    * by u+wx so that the subdirectories can always be created.
//    */
//   mode = 0777 & ~umask(0);
//   dir_mode = mode | S_IWUSR | S_IXUSR;
//
//   pflag = 0;
//   while ((ch = getopt(argc, argv, "m:p")) != -1)
//     switch (ch) {
//     case 'p':
//       pflag = 1;
//       break;
//     case 'm':
//       if ((set = setmode(optarg)) == NULL)
//         errx(1, "invalid file mode: %s", optarg);
//       mode = getmode(set, S_IRWXU | S_IRWXG | S_IRWXO);
//       free(set);
//       break;
//     default:
//       usage();
//     }
//   argc -= optind;
//   argv += optind;
//
//   if ((mode & (S_ISUID | S_ISGID | S_ISTXT)) == 0) {
//     if (pledge("stdio rpath cpath fattr", NULL) == -1)
//       err(1, "pledge");
//   }
//
//   if (*argv == NULL)
//     usage();
//
//   for (exitval = 0; *argv != NULL; ++argv) {
//     char *slash;
//
//     /* Remove trailing slashes, per POSIX. */
//     slash = strrchr(*argv, '\0');
//     while (--slash > *argv && *slash == '/')
//       *slash = '\0';
//
//     if (pflag) {
//       rv = mkpath(*argv, mode, dir_mode);
//     } else {
//       rv = mkdir(*argv, mode);
//       /*
//        * The mkdir() and umask() calls both honor only the
//        * low nine bits, so if you try to set a mode including
//        * the sticky, setuid, setgid bits you lose them. Don't
//        * do this unless the user has specifically requested
//        * a mode as chmod will (obviously) ignore the umask.
//        */
//       if (rv == 0 && mode > 0777)
//         rv = chmod(*argv, mode);
//     }
//     if (rv == -1) {
//       warn("%s", *argv);
//       exitval = 1;
//     }
//   }
//   return exitval;
// }
//
// int build(char *path, mode_t omode) {
//   struct stat sb;
//   mode_t numask, oumask;
//   int first, last, retval;
//   char *p;
//
//   p = path;
//   oumask = 0;
//   retval = 0;
//   if (p[0] == '/') /* Skip leading '/'. */
//     ++p;
//   for (first = 1, last = 0; !last; ++p) {
//     if (p[0] == '\0')
//       last = 1;
//     else if (p[0] != '/')
//       continue;
//     *p = '\0';
//     if (!last && p[1] == '\0')
//       last = 1;
//     if (first) {
//       /*
//        * POSIX 1003.2:
//        * For each dir operand that does not name an existing
//        * directory, effects equivalent to those cased by the
//        * following command shall occcur:
//        *
//        * mkdir -p -m $(umask -S),u+wx $(dirname dir) &&
//        *    mkdir [-m mode] dir
//        *
//        * We change the user's umask and then restore it,
//        * instead of doing chmod's.
//        */
//       oumask = umask(0);
//       numask = oumask & ~(S_IWUSR | S_IXUSR);
//       (void)umask(numask);
//       first = 0;
//     }
//     if (last)
//       (void)umask(oumask);
//     if (mkdir(path, last ? omode : S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
//       if (errno == EEXIST || errno == EISDIR) {
//         if (stat(path, &sb) < 0) {
//           warn("%s", path);
//           retval = 1;
//           break;
//         } else if (!S_ISDIR(sb.st_mode)) {
//           if (last)
//             errno = EEXIST;
//           else
//             errno = ENOTDIR;
//           warn("%s", path);
//           retval = 1;
//           break;
//         }
//       } else {
//         warn("%s", path);
//         retval = 1;
//         break;
//       }
//     } else if (vflag)
//       printf("%s\n", path);
//     if (!last)
//       *p = '/';
//   }
//   if (!first && !last)
//     (void)umask(oumask);
//   return (retval);
// }
//
// #include <sys/cdefs.h>
//
// #include <sys/types.h>
// #include <sys/stat.h>
//
// #include <err.h>
// #include <errno.h>
// #include <libgen.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sysexits.h>
// #include <unistd.h>
//
// static int  build(char *, mode_t);
// static void usage(void);
//
// int vflag;
//
// int
// main(int argc, char *argv[])
// {
//     int ch, exitval, success, pflag;
//     mode_t omode;
//     void *set = NULL;
//     char *mode;
//
//     omode = pflag = 0;
//     mode = NULL;
//     while ((ch = getopt(argc, argv, "m:pv")) != -1)
//         switch(ch) {
//         case 'm':
//             mode = optarg;
//             break;
//         case 'p':
//             pflag = 1;
//             break;
//         case 'v':
//             vflag = 1;
//             break;
//         case '?':
//         default:
//             usage();
//         }
//
//     argc -= optind;
//     argv += optind;
//     if (argv[0] == NULL)
//         usage();
//
//     if (mode == NULL) {
//         omode = S_IRWXU | S_IRWXG | S_IRWXO;
//     } else {
//         if ((set = setmode(mode)) == NULL)
//             errx(1, "invalid file mode: %s", mode);
//         omode = getmode(set, S_IRWXU | S_IRWXG | S_IRWXO);
//         free(set);
//     }
//
//     for (exitval = 0; *argv != NULL; ++argv) {
//         success = 1;
//         if (pflag) {
//             if (build(*argv, omode))
//                 success = 0;
//         } else if (mkdir(*argv, omode) < 0) {
//             if (errno == ENOTDIR || errno == ENOENT)
//                 warn("%s", dirname(*argv));
//             else
//                 warn("%s", *argv);
//             success = 0;
//         } else if (vflag)
//             (void)printf("%s\n", *argv);
//
//         if (!success)
//             exitval = 1;
//         /*
//          * The mkdir() and umask() calls both honor only the low
//          * nine bits, so if you try to set a mode including the
//          * sticky, setuid, setgid bits you lose them.  Don't do
//          * this unless the user has specifically requested a mode,
//          * as chmod will (obviously) ignore the umask.
//          */
//         if (success && mode != NULL && chmod(*argv, omode) == -1) {
//             warn("%s", *argv);
//             exitval = 1;
//         }
//     }
//     exit(exitval);
// }
