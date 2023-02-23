//-------------------------------------------------------------------
// Copyright 2020 Carl-Johan Seger
// SPDX-License-Identifier: Apache-2.0
//-------------------------------------------------------------------

#include "aux.h"
#include <cstdio>
#include <iostream>
using namespace std;

int main(int, char **argv) {
  CircuitGroup *circuit = parse(argv[1]);

  cout << circuit->sign() << endl;

  delete circuit;
  return 0;
}
