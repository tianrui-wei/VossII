//-------------------------------------------------------------------
// Copyright 2020 Carl-Johan Seger
// SPDX-License-Identifier: Apache-2.0
//-------------------------------------------------------------------

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  struct sigaction sa = {0};

  sa.sa_handler = SIG_IGN;
  sigaction(SIGINT, &sa, 0);
  struct sigaction sb = {0};
  sb.sa_handler = SIG_IGN;
  sigaction(SIGTRAP, &sb, 0);

  if (argc > 1) {
    execvp(argv[1], argv + 1);
    perror("execv");
  } else {
    fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
  }
  return 1;
}
