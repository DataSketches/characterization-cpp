/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "kll_accuracy_profile.h"

#include <iostream>
#include <algorithm>
#include <cmath>

namespace sketches {

/*
 * Computes the next larger integer point in the power series
 * <i>point = 2<sup>( i / ppo )</sup></i> given the current point in the series.
 * For illustration, this can be used in a loop as follows:
 *
 * <pre>{@code
 *     int maxP = 1024;
 *     int minP = 1;
 *     int ppo = 2;
 *
 *     for (int p = minP; p <= maxP; p = pwr2LawNext(ppo, p)) {
 *       System.out.print(p + " ");
 *     }
 *     //generates the following series:
 *     //1 2 3 4 6 8 11 16 23 32 45 64 91 128 181 256 362 512 724 1024
 * }</pre>
 *
 * param ppo Points-Per-Octave, or the number of points per integer powers of 2 in the series.
 * param curPoint the current point of the series. Must be &ge; 1.
 * returns the next point in the power series.
 */
size_t pwr_2_law_next(size_t ppo, size_t cur_point) {
  const size_t cur((cur_point < 1) ? 1 : cur_point);
  size_t gi(round(log2(cur) * ppo)); //current generating index
  size_t next;
  do {
    next = (size_t) round(pow(2.0, (double) ++gi / ppo));
  } while ( next <= cur_point);
  return next;
}

/*
 * Counts the actual number of plotting points between lgStart and lgEnd assuming the given PPO.
 * This is not a simple linear function due to points that may be skipped in the low range.
 * param lgStart Log2 of the starting value
 * param lgEnd Log2 of the ending value
 * param ppo the number of logarithmically evenly spaced points per octave.
 * returns the actual number of plotting points between lgStart and lgEnd.
 */
size_t count_points(size_t lg_start, size_t lg_end, size_t ppo) {
  size_t p(1 << lg_start);
  const size_t end(1 << lg_end);
  size_t count(0);
  while (p <= end) {
    p = pwr_2_law_next(ppo, p);
    count++;
  }
  return count;
}

void kll_accuracy_profile::run() {
  const unsigned lg_min(0);
  const unsigned lg_max(23);
  const unsigned ppo(16);
  const unsigned num_trials(100);
  const unsigned error_pct(99);

  double rank_errors[num_trials];

  unsigned max_len(1 << lg_max);
  float* values = new float[max_len];

  const unsigned num_steps = count_points(lg_min, lg_max, ppo);
  unsigned stream_length(1 << lg_min);
  for (unsigned i = 0; i < num_steps; i++) {
    for (unsigned i = 0; i < stream_length; i++) values[i] = i;

    for (unsigned t = 0; t < num_trials; t++) {
      const double maxRankErrorInTrial = run_trial(values, stream_length);
      rank_errors[t] = maxRankErrorInTrial;
    }

    std::sort(&rank_errors[0], &rank_errors[num_trials]);
    const unsigned error_pct_index = num_trials * error_pct / 100;
    const double rank_error = rank_errors[error_pct_index];

    std::cout << stream_length << "\t" << rank_error * 100 << std::endl;

    stream_length = pwr_2_law_next(ppo, stream_length);
  }
  delete [] values;
}

} /* namespace sketches */
