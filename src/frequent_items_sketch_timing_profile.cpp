/*
 * Copyright 2019, Verizon Media.
 * Licensed under the terms of the Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "frequent_items_sketch_timing_profile.h"
#include "characterization_utils.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <sstream>

#include <frequent_items_sketch.hpp>

namespace datasketches {

void frequent_items_sketch_timing_profile::run() {
  std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
  std::geometric_distribution<long long> geometric_distribution(0.01);

  const unsigned lg_min_stream_len = 0;
  const unsigned lg_max_stream_len = 23;
  const unsigned ppo = 16;

  const unsigned lg_max_trials = 16;
  const unsigned lg_min_trials = 8;

  const unsigned lg_max_sketch_size = 10;

  size_t stream_length = 1 << lg_min_stream_len;
  while (stream_length <= 1 << lg_max_stream_len) {
    std::chrono::nanoseconds build_time_ns(0);
    std::chrono::nanoseconds update_time_ns(0);
    std::chrono::nanoseconds serialize_time_ns(0);
    std::chrono::nanoseconds deserialize_time_ns(0);
    size_t size_bytes = 0;

    const size_t num_trials = get_num_trials(stream_length, lg_min_stream_len, lg_max_stream_len, lg_min_trials, lg_max_trials);

    std::unique_ptr<frequent_items_sketch<long long>> sketches[num_trials];

    const auto start_build(std::chrono::high_resolution_clock::now());
    for (size_t i = 0; i < num_trials; i++) {
      sketches[i] = std::make_unique<frequent_items_sketch<long long>>(lg_max_sketch_size);
    }
    const auto finish_build(std::chrono::high_resolution_clock::now());
    build_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_build - start_build);

    long long* items = new long long[stream_length];
    for (size_t i = 0; i < stream_length; i++) items[i] = geometric_distribution(generator);

    const auto start_update(std::chrono::high_resolution_clock::now());
    for (size_t i = 0; i < num_trials; i++) {
      for (size_t j = 0; j < stream_length; j++) {
        sketches[i]->update(items[j]);
      }
    }
    const auto finish_update(std::chrono::high_resolution_clock::now());
    update_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_update - start_update);

    delete [] items;

    std::stringstream s(std::ios::in | std::ios::out | std::ios::binary);
    auto start_serialize(std::chrono::high_resolution_clock::now());
    for (size_t i = 0; i < num_trials; i++) {
      sketches[i]->serialize(s);
    }
    const auto finish_serialize(std::chrono::high_resolution_clock::now());
    serialize_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_serialize - start_serialize);

    const auto start_deserialize(std::chrono::high_resolution_clock::now());
    for (size_t i = 0; i < num_trials; i++) {
      auto deserialized_sketch = frequent_items_sketch<long long>::deserialize(s);
    }
    const auto finish_deserialize(std::chrono::high_resolution_clock::now());
    deserialize_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finish_deserialize - start_deserialize);

    size_bytes += s.tellp();

    std::cout << stream_length << "\t"
        << num_trials << "\t"
        << (double) build_time_ns.count() / num_trials << "\t"
        << (double) update_time_ns.count() / num_trials / stream_length << "\t"
        << (double) serialize_time_ns.count() / num_trials << "\t"
        << (double) deserialize_time_ns.count() / num_trials << "\t"
        << (double) size_bytes / num_trials << "\t"
        << std::endl;

    stream_length = pwr_2_law_next(ppo, stream_length);
  }
}

} /* namespace datasketches */
