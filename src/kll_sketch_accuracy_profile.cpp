/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "kll_sketch_accuracy_profile.h"

#include <algorithm>
#include <random>
#include <chrono>

#include <kll_sketch.hpp>

namespace sketches {

double kll_sketch_accuracy_profile::run_trial(float* values, unsigned stream_length) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::shuffle(values, values + stream_length, std::default_random_engine(seed));

  sketches::kll_sketch sketch;
  for (size_t i = 0; i < stream_length; i++) sketch.update(values[i]);

  double max_rank_error = 0;
  for (size_t i = 0; i < stream_length; i++) {
    double true_rank = (double) i / stream_length;
    double est_rank = sketch.get_rank(i);
    max_rank_error = std::max(max_rank_error, abs(true_rank - est_rank));
  }

  return max_rank_error;
}

} /* namespace sketches */
