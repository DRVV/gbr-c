#include <stdio.h>
#include "for_debug.h"

void halt(void) {
  fflush(stdout);
  getchar();
}
