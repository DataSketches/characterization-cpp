/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include <iostream>
#include <cstring>

#include "kll_sketch_accuracy_profile.h"
#include "kll_sketch_timing_profile.h"
#include "kll_merge_accuracy_profile.h"

int main(int argc, char **argv) {
  if (argc == 2) {
    if (strcmp(argv[1], "kll-accuracy") == 0) {
      sketches::kll_sketch_accuracy_profile profile;
      profile.run();
    } else if (strcmp(argv[1], "kll-timing") == 0) {
      sketches::kll_sketch_timing_profile profile;
      profile.run();
    } else if (strcmp(argv[1], "kll-merge-accuracy") == 0) {
      sketches::kll_merge_accuracy_profile profile;
      profile.run();
    } else {
      std::cerr << "Unsupported command " << argv[1] << std::endl;
    }
  } else {
    std::cerr << "One parameter expected: kll-accuracy, kll-timing or kll-merge-accuracy" << std::endl;
  }
  return 0;
}
