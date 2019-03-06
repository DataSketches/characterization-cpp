/*
 * Copyright 2019, Verizon Media.
 * Licensed under the terms of the Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "characterization_utils.h"

#include <cstddef>
#include <cmath>

namespace datasketches {

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

size_t get_num_trials(size_t x, size_t lg_min_x, size_t lg_max_x, size_t lg_min_trials, size_t lg_max_trials) {
  const double slope((double) (lg_max_trials - lg_min_trials) / ((int) lg_min_x - (int) lg_max_x));
  const double lg_trials((slope * log2(x)) + lg_max_trials);
  return (size_t) pow(2, lg_trials);
}

} /* namespace datasketches */
