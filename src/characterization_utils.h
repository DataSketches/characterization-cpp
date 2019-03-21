/*
 * Copyright 2019, Verizon Media.
 * Licensed under the terms of the Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef CHARACTERIZATION_UTIL_H_
#define CHARACTERIZATION_UTIL_H_

#include <cstddef>

namespace datasketches {

size_t pwr_2_law_next(size_t ppo, size_t cur_point);
size_t count_points(size_t lg_start, size_t lg_end, size_t ppo);
size_t get_num_trials(size_t x, size_t lg_min_x, size_t lg_max_x, size_t lg_min_trials, size_t lg_max_trials);

} /* namespace datasketches */

#endif /* CHARACTERIZATION_UTIL_H_ */
