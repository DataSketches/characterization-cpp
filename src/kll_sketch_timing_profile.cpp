/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "kll_sketch_timing_profile.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <sstream>

#include <kll_sketch.hpp>

namespace sketches {

size_t pwr_2_law_next(size_t ppo, size_t cur_point);
size_t count_points(size_t lg_start, size_t lg_end, size_t ppo);

size_t get_num_trials(size_t x, size_t lg_min_x, size_t lg_max_x, size_t lg_min_trials, size_t lg_max_trials) {
  const double slope((double) (lg_max_trials - lg_min_trials) / ((int) lg_min_x - (int) lg_max_x));
  const double lg_trials((slope * log2(x)) + lg_max_trials);
  return (size_t) pow(2, lg_trials);
}

kll_sketch_timing_profile::kll_sketch_timing_profile() {
  // TODO Auto-generated constructor stub

}

kll_sketch_timing_profile::~kll_sketch_timing_profile() {
  // TODO Auto-generated destructor stub
}

void kll_sketch_timing_profile::run() {
  const size_t lg_min_stream_len(0);
  const size_t lg_max_stream_len(23);
  const size_t ppo(16);

  const size_t lg_max_trials(16);
  const size_t lg_min_trials(6);

  const size_t num_queries(20);

  std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<float> distribution(0.0, 1.0);

  std::cout << "Stream\tTrials\tBuild\tUpdate\tQuant\tQuants\tRank\tCDF\tSer\tDeser\tItems\tSize" << std::endl;

  size_t max_len(1 << lg_max_stream_len);
  float* values = new float[max_len];

  float rank_query_values[num_queries];
  for (size_t i = 0; i < num_queries; i++) rank_query_values[i] = distribution(generator);
  std::sort(&rank_query_values[0], &rank_query_values[num_queries]);

  double quantile_query_values[num_queries];
  for (size_t i = 0; i < num_queries; i++) quantile_query_values[i] = distribution(generator);

  size_t stream_length(1 << lg_min_stream_len);
  while (stream_length <= (1 << lg_max_stream_len)) {

    std::chrono::nanoseconds build_time_ns(0);
    std::chrono::nanoseconds update_time_ns(0);
    std::chrono::nanoseconds get_quantile_time_ns(0);
    std::chrono::nanoseconds get_quantiles_time_ns(0);
    std::chrono::nanoseconds get_rank_time_ns(0);
    std::chrono::nanoseconds get_cdf_time_ns(0);
    std::chrono::nanoseconds serialize_time_ns(0);
    std::chrono::nanoseconds deserialize_time_ns(0);
    size_t num_retained(0);
    size_t size_bytes(0);

    const size_t num_trials = get_num_trials(stream_length, lg_min_stream_len, lg_max_stream_len, lg_min_trials, lg_max_trials);
    for (size_t i = 0; i < num_trials; i++) {
      for (size_t i = 0; i < stream_length; i++) values[i] = distribution(generator);

      auto start_build(std::chrono::high_resolution_clock::now());
      sketches::kll_sketch sketch;
      auto finish_build(std::chrono::high_resolution_clock::now());
      build_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_build - start_build);

      auto start_update(std::chrono::high_resolution_clock::now());
      for (size_t i = 0; i < stream_length; i++) sketch.update(values[i]);
      auto finish_update(std::chrono::high_resolution_clock::now());
      update_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_update - start_update);

      auto start_get_quantile(std::chrono::high_resolution_clock::now());
      for (size_t i = 0; i < num_queries; i++) sketch.get_quantile(quantile_query_values[i]);
      auto finish_get_quantile(std::chrono::high_resolution_clock::now());
      get_quantile_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_quantile - start_get_quantile);

      auto start_get_quantiles(std::chrono::high_resolution_clock::now());
      sketch.get_quantiles(quantile_query_values, num_queries);
      auto finish_get_quantiles(std::chrono::high_resolution_clock::now());
      get_quantiles_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_quantiles - start_get_quantiles);

      auto start_get_rank(std::chrono::high_resolution_clock::now());
      for (size_t i = 0; i < num_queries; i++) sketch.get_rank(rank_query_values[i]);
      auto finish_get_rank(std::chrono::high_resolution_clock::now());
      get_rank_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_rank - start_get_rank);

      auto start_get_cdf(std::chrono::high_resolution_clock::now());
      sketch.get_CDF(rank_query_values, num_queries);
      auto finish_get_cdf(std::chrono::high_resolution_clock::now());
      get_cdf_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_get_cdf - start_get_cdf);

      std::stringstream s(std::ios::in | std::ios::out | std::ios::binary);
      auto start_serialize(std::chrono::high_resolution_clock::now());
      sketch.serialize(s);
      auto finish_serialize(std::chrono::high_resolution_clock::now());
      serialize_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_serialize - start_serialize);

      auto start_deserialize(std::chrono::high_resolution_clock::now());
      auto sketch_ptr(sketches::kll_sketch::deserialize(s));
      auto finish_deserialize(std::chrono::high_resolution_clock::now());
      deserialize_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_deserialize - start_deserialize);

      num_retained += sketch.get_num_retained();
      size_bytes += s.tellp();
    }
    std::cout << stream_length << "\t"
        << num_trials << "\t"
        << (double) build_time_ns.count() / num_trials << "\t"
        << (double) update_time_ns.count() / num_trials / stream_length << "\t"
        << (double) get_quantile_time_ns.count() / num_trials / num_queries << "\t"
        << (double) get_quantiles_time_ns.count() / num_trials / num_queries << "\t"
        << (double) get_rank_time_ns.count() / num_trials / num_queries << "\t"
        << (double) get_cdf_time_ns.count() / num_trials / num_queries << "\t"
        << (double) serialize_time_ns.count() / num_trials << "\t"
        << (double) deserialize_time_ns.count() / num_trials << "\t"
        << num_retained / num_trials << "\t"
        << size_bytes / num_trials << std::endl;
    stream_length = pwr_2_law_next(ppo, stream_length);
  }
  delete [] values;
}

} /* namespace sketches */
