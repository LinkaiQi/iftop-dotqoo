/*
 * tui.c:
 *
 * Based on ui.c from the original iftop sources.
 *
 * This user interface does not make use of curses. Instead, it prints its
 * output to STDOUT. This output is activated by providing the '-t' flag.
 *
 */

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(HAVE_TERMIOS_H)
#include <termios.h>
#elif defined(HAVE_SGTTY_H) && defined(TIOCSETP)
#include <sys/ioctl.h>
#include <sgtty.h>
#elif defined(HAVE_TERMIO_H)
#include <sys/ioctl.h>
#include <termio.h>
#else
#include <stdlib.h>
#endif

#include "sorted_list.h"
#include "options.h"
#include "ui_common.h"

/* Width of the host column in the output */
#define PRINT_WIDTH 40


/*
 * UI print function
 */
//void tui_print() { /* function */}


/*
 * Text interface data structure initializations.
 */
// void tui_init()


/*
 * Tick function indicating screen refresh
 */
// void tui_tick(int print)

/*
 * Main UI loop. Code any interactive character inputs here.
 */
void loop() {
  int i;
  extern sig_atomic_t foad;

#if defined(HAVE_TERMIOS_H)
  struct termios new_termios, old_termios;

  tcgetattr(STDIN_FILENO, &old_termios);
  new_termios = old_termios;
  new_termios.c_lflag &= ~(ICANON|ECHO);
  new_termios.c_cc[VMIN] = 1;
  new_termios.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
#elif defined(HAVE_SGTTY_H) && defined(TIOCSETP)
  struct sgttyb new_tty, old_tty;

  ioctl(STDIN_FILENO, TIOCGETP, &old_tty);
  new_tty = old_tty;
  new_tty.sg_flags &= ~(ICANON|ECHO);
  ioctl(STDIN_FILENO, TIOCSETP, &new_tty);
#elif defined(HAVE_TERMIO_H)
  struct termio new_termio, old_termio;

  ioctl(0, TCGETA, &old_termio);
  new_termio = old_termio;
  new_termio.c_lflag &= ~(ICANON|ECHO);
  new_termio.c_cc[VMIN] = 1;
  new_termio.c_cc[VTIME] = 0;
  ioctl(0, TCSETA, &new_termio);
#else
  system("/bin/stty cbreak -echo >/dev/null 2>&1");
#endif

  while ((i = getchar()) != 'q' && foad == 0) {
    switch (i) {
      case 'u':
        tick(1);
        break;
      /*
      case 'n':
        options.dnsresolution ^= 1;
        printf("DNS resolution is %s.\n\n", options.dnsresolution ? "ON" : "OFF");
        tick(1);
        break;
      case 'N':
        options.portresolution ^= 1;
        printf("Port resolution is %s.\n\n", options.portresolution ? "ON" : "OFF");
        tick(1);
        break;
      case 's':
        options.aggregate_src ^= 1;
        printf("%s source host\n\n", options.aggregate_src ? "Hide" : "Show");
        tick(1);
        break;
      case 'd':
        options.aggregate_dest ^= 1;
        printf("%s destination host\n\n", options.aggregate_dest ? "Hide" : "Show");
        tick(1);
        break;
      case 'S':
        if (options.showports == OPTION_PORTS_OFF) {
          options.showports = OPTION_PORTS_SRC;
        }
        else if (options.showports == OPTION_PORTS_DEST) {
          options.showports = OPTION_PORTS_ON;
        }
        else if(options.showports == OPTION_PORTS_ON) {
          options.showports = OPTION_PORTS_DEST;
        }
        else {
          options.showports = OPTION_PORTS_OFF;
        }
        printf("Showing ports:%s%s%s%s.\n\n",
          options.showports == OPTION_PORTS_SRC ? " src" : "",
          options.showports == OPTION_PORTS_DEST ? " dest" : "",
          options.showports == OPTION_PORTS_ON ? " both" : "",
          options.showports == OPTION_PORTS_OFF ? " none" : "");
        tick(1);
        break;
      case 'D':
        if (options.showports == OPTION_PORTS_OFF) {
          options.showports = OPTION_PORTS_DEST;
        }
        else if (options.showports == OPTION_PORTS_SRC) {
          options.showports = OPTION_PORTS_ON;
        }
        else if(options.showports == OPTION_PORTS_ON) {
          options.showports = OPTION_PORTS_SRC;
        }
        else {
          options.showports = OPTION_PORTS_OFF;
        }
        printf("Showing ports:%s%s%s%s.\n\n",
          options.showports == OPTION_PORTS_SRC ? " src" : "",
          options.showports == OPTION_PORTS_DEST ? " dest" : "",
          options.showports == OPTION_PORTS_ON ? " both" : "",
          options.showports == OPTION_PORTS_OFF ? " none" : "");
        tick(1);
        break;
      case 'p':
        options.showports =
         (options.showports == OPTION_PORTS_OFF) ?
          OPTION_PORTS_ON :
          OPTION_PORTS_OFF;
        printf("Showing ports:%s%s%s%s.\n\n",
          options.showports == OPTION_PORTS_SRC ? " src" : "",
          options.showports == OPTION_PORTS_DEST ? " dest" : "",
          options.showports == OPTION_PORTS_ON ? " both" : "",
          options.showports == OPTION_PORTS_OFF ? " none" : "");
        tick(1);
        break;
      case 'P':
        options.paused ^= 1;
        if (options.paused) {
          printf("Pausing... press 'P' again to continue.\n");
        }
	    else {
	      printf("Continuing.\n\n");
	      tick(1);
	    }
	    break;
      case 'o':
        options.freezeorder ^= 1;
        printf("Order %s.\n\n", options.freezeorder ? "frozen" : "unfrozen");
	    tick(1);
	    break;
      case '1':
        options.sort = OPTION_SORT_DIV1;
        printf("Sorting by column 1.\n\n");
	    tick(1);
        break;
      case '2':
        options.sort = OPTION_SORT_DIV2;
        printf("Sorting by column 2.\n\n");
        tick(1);
        break;
      case '3':
        options.sort = OPTION_SORT_DIV3;
        printf("Sorting by column 3.\n\n");
        tick(1);
        break;
      case '<':
        options.sort = OPTION_SORT_SRC;
        printf("Sorting by column source.\n\n");
        tick(1);
        break;
      case '>':
        options.sort = OPTION_SORT_DEST;
        printf("Sorting by column destination.\n\n");
        tick(1);
        break;
      */
      default:
        break;
    }
  }

#if defined(HAVE_TERMIOS_H)
  tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
#elif defined(HAVE_SGTTY_H) && defined(TIOCSETP)
  ioctl(0, TIOCSETP, &old_tty);
#elif defined(HAVE_TERMIO_H)
  ioctl(0, TCSETA, &old_termio);
#else
  system("/bin/stty -cbreak echo >/dev/null 2>&1");
#endif
}
