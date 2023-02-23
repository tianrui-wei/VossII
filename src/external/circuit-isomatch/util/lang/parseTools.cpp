//-------------------------------------------------------------------
// Copyright 2020 Carl-Johan Seger
// SPDX-License-Identifier: Apache-2.0
//-------------------------------------------------------------------

#include "parseTools.h"

using namespace std;

namespace parseTools {

void makeGroup(CircuitGroup *stub, const std::vector<std::string> &inputs,
               const std::vector<std::string> &outputs,
               const std::vector<CircuitTree *> &parts) {
  for (auto inp = inputs.begin(); inp != inputs.end(); ++inp)
    stub->addInput(*inp, stub->wireManager()->wire(*inp));

  for (auto out = outputs.begin(); out != outputs.end(); ++out)
    stub->addOutput(*out, stub->wireManager()->wire(*out));

  for (auto part : parts) {
    stub->addChild(part);
  }
}

} // namespace parseTools
