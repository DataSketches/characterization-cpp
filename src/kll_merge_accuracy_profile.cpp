/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "kll_merge_accuracy_profile.h"

#include <algorithm>
#include <random>
#include <chrono>

#include <kll_sketch.hpp>

namespace datasketches {

double kll_merge_accuracy_profile::run_trial(float* values, unsigned stream_length) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::shuffle(values, values + stream_length, std::default_random_engine(seed));

  const unsigned num_sketches(8);
  std::unique_ptr<kll_sketch<float>> sketches[num_sketches];
  for (unsigned i = 0; i < num_sketches; i++) {
    sketches[i] = std::unique_ptr<kll_sketch<float>>(new kll_sketch<float>());
  }

  unsigned j(0);
  for (size_t i = 0; i < stream_length; i++) {
    sketches[j]->update(values[i]);
    j++;
    if (j == num_sketches) j = 0;
  }

  kll_sketch<float> sketch_tmp(32*200);
  for (unsigned i = 0; i < num_sketches; i++) sketch_tmp.merge(*sketches[i]);

  kll_sketch<float> sketch;
  sketch.merge(sketch_tmp);

  double max_rank_error = 0;
  for (size_t i = 0; i < stream_length; i++) {
    double true_rank = (double) i / stream_length;
    double est_rank = sketch.get_rank(i);
    max_rank_error = std::max(max_rank_error, abs(true_rank - est_rank));
  }

  return max_rank_error;
}

} /* namespace datasketches */
