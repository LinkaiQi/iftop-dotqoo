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
#include <time.h>

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
        tick(0, time(NULL));
        break;
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
